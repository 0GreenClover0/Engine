#include "DialoguePromptController.h"
#include "AK/Math.h"
#include "DialogueObject.h"
#include "Entity.h"
#include "Game/LevelController.h"
#include "Globals.h"
#include "Input.h"
#include "Sound.h"

#include <algorithm>

#if EDITOR
#include "imgui_extensions.h"
#include "imgui_stdlib.h"
#endif

std::shared_ptr<DialoguePromptController> DialoguePromptController::create()
{
    auto prompt = std::make_shared<DialoguePromptController>(AK::Badge<DialoguePromptController> {});
    return prompt;
}

DialoguePromptController::DialoguePromptController(AK::Badge<DialoguePromptController>)
{
}

void DialoguePromptController::awake()
{
    upper_text.lock()->color = 0xfff4ecf8;
    upper_text.lock()->font_size = 30;
    middle_text.lock()->color = 0xfff4ecf8;
    middle_text.lock()->font_size = 30;
    lower_text.lock()->color = 0xfff4ecf8;
    lower_text.lock()->font_size = 30;

    set_can_tick(true);
}

void DialoguePromptController::update()
{
    if (m_interpolation_value > 0.99f && m_currently_played_sound == nullptr)
    {
        m_currently_played_sound = Sound::play_sound(dialogue_objects[m_currently_played_content].sound_path);
        m_currently_played_sound->set_volume(0.65f);
    }

    if (m_currently_played_sound != nullptr && m_currently_played_sound->has_finished()
        && dialogue_objects[m_currently_played_content].auto_end)
    {
        LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::DialogEnded);
        end_content();
    }

    realign_lines();

    if (!m_perform_panel_move)
        return;

    auto const panel_entity = panel_parent.lock();
    glm::vec3 v = panel_entity->transform->get_local_position();

    switch (m_interpolation_mode)
    {
    case InterpolationMode::Show:
        if (m_perform_panel_move)
        {
            if (m_interpolation_value < 0.99f)
            {
                m_interpolation_value += static_cast<float>(delta_time);
                v.y = AK::Math::ease_in_out_elastic(m_interpolation_value) - 1.8f;
            }
            else
            {
                m_interpolation_value = 1.0f;
                v.y = AK::Math::ease_in_out_elastic(m_interpolation_value) - 1.8f;
                m_perform_panel_move = false;
            }
        }
        break;

    case InterpolationMode::Hide:
        if (m_interpolation_value > 0.01f)
        {
            m_interpolation_value -= static_cast<float>(delta_time);
            v.y = AK::Math::ease_in_out_elastic(m_interpolation_value) - 1.8f;
        }
        else
        {
            m_interpolation_value = 0.0f;
            v.y = AK::Math::ease_in_out_elastic(m_interpolation_value) - 1.8f;
            m_perform_panel_move = false;
        }
        break;
    }

    panel_entity->transform->set_local_position(v);
}

#if EDITOR
void DialoguePromptController::draw_editor()
{
    Component::draw_editor();

    realign_lines();

    if (ImGui::Button("Play"))
    {
        play_content(0);
    }

    ImGuiEx::draw_ptr("Panel Reference", dialogue_panel);
    ImGuiEx::draw_ptr("Parent Reference", panel_parent);
    ImGui::Separator();
    ImGuiEx::draw_ptr("Upper text", upper_text);
    ImGuiEx::draw_ptr("Middle text", middle_text);
    ImGuiEx::draw_ptr("Lower text", lower_text);

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    // ImGui code for vector of DialogueObject
    ImGui::Text("Dialogue Objects:");

    for (int i = 0; i < dialogue_objects.size(); i++)
    {
        ImGui::PushID(i);

        // Display the index and a remove button
        ImGui::Text("Index: %d", i);
        if (ImGui::Button("Remove"))
        {
            dialogue_objects.erase(dialogue_objects.begin() + i);
            ImGui::PopID();
            continue;
        }

        // Display the individual strings of the DialogueObject structure
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Structure Entry"))
        {
            ImGui::InputText("Upper Line", &dialogue_objects[i].upper_line);
            ImGui::InputText("Middle Line", &dialogue_objects[i].middle_line);
            ImGui::InputText("Lower Line", &dialogue_objects[i].lower_line);
            ImGui::Spacing();
            ImGui::InputText("Sound Path", &dialogue_objects[i].sound_path);
            ImGui::Checkbox("Auto End", &dialogue_objects[i].auto_end);

            ImGui::TreePop();
        }

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::PopID();
    }

    ImGui::Spacing();

    if (ImGui::Button("Add New Entry"))
    {
        dialogue_objects.emplace_back();
    }
}
#endif

void DialoguePromptController::show_or_hide_panel(InterpolationMode const& show)
{
    m_perform_panel_move = true;
    m_interpolation_mode = show;

    switch (show)
    {
    case InterpolationMode::Show:
        m_interpolation_value = 0.01f;
        break;

    case InterpolationMode::Hide:
        m_interpolation_value = 0.99f;
        break;
    }
}

u8 DialoguePromptController::get_empty_lines() const
{
    u8 empty_lines = 0;
    std::vector const lines = {upper_text, middle_text, lower_text};
    for (u8 i = 0; i < lines.size(); i++)
    {
        if (lines[i].lock()->text.empty())
            empty_lines++;
    }

    return empty_lines;
}

void DialoguePromptController::realign_lines() const
{
    std::weak_ptr<ScreenText> empty = {};
    std::weak_ptr<ScreenText> new_upper = {};
    std::weak_ptr<ScreenText> new_lower = {};

    if (get_empty_lines() == 1)
    {
        std::vector lines = {upper_text, middle_text, lower_text};
        for (u8 i = 0; i < lines.size(); i++)
        {
            if (lines[i].lock()->text.empty())
            {
                empty = lines[i];
                lines.erase(lines.begin() + i);
                break;
            }
        }

        new_upper = lines[0];
        new_lower = lines[1];

        glm::vec3 upper_position = new_upper.lock()->entity->transform->get_local_position();
        new_upper.lock()->entity->transform->set_local_position({upper_position.x, 0.18f, upper_position.z});

        glm::vec3 lower_position = new_lower.lock()->entity->transform->get_local_position();
        new_lower.lock()->entity->transform->set_local_position({lower_position.x, -0.18f, lower_position.z});
    }
    else
    {
        glm::vec3 upper_position = upper_text.lock()->entity->transform->get_local_position();
        upper_text.lock()->entity->transform->set_local_position({upper_position.x, 0.36f, upper_position.z});

        glm::vec3 middle_position = middle_text.lock()->entity->transform->get_local_position();
        middle_text.lock()->entity->transform->set_local_position({middle_position.x, 0.0f, middle_position.z});

        glm::vec3 lower_position = lower_text.lock()->entity->transform->get_local_position();
        lower_text.lock()->entity->transform->set_local_position({lower_position.x, -0.36f, lower_position.z});
    }
}

void DialoguePromptController::play_content(u16 const vector_index)
{
    m_currently_played_content = vector_index;
    DialogueObject const dialogue = dialogue_objects[vector_index];
    bool const auto_end = dialogue_objects[vector_index].auto_end;

    upper_text.lock()->set_text(dialogue.upper_line);
    middle_text.lock()->set_text(dialogue.middle_line);
    lower_text.lock()->set_text(dialogue.lower_line);

    show_or_hide_panel(InterpolationMode::Show);
}

void DialoguePromptController::end_content()
{
    if (m_currently_played_content < 0)
        return;

    DialogueObject const dialogue = dialogue_objects[m_currently_played_content];

    if (m_currently_played_sound != nullptr)
        m_currently_played_sound->stop_with_fade(500);

    show_or_hide_panel(InterpolationMode::Hide);
    m_currently_played_content = -1;
    m_currently_played_sound = nullptr;
}
