#include "FloeButton.h"

#include "Collider2D.h"
#include "Entity.h"
#include "ExampleDynamicText.h"
#include "Game/GameController.h"
#include "Game/LighthouseLight.h"

std::shared_ptr<FloeButton> FloeButton::create()
{
    auto floe = std::make_shared<FloeButton>(AK::Badge<FloeButton> {});
    return floe;
}

FloeButton::FloeButton(AK::Badge<FloeButton>)
{
}

void FloeButton::awake()
{
    Component::awake();
    set_can_tick(true);
}

void FloeButton::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    Component::on_trigger_enter(other);

    std::weak_ptr<LighthouseLight> lighthouse_light = {};
    if (auto const l = other->entity->get_component<LighthouseLight>())
    {
        lighthouse_light = l;

        switch (floe_button_type)
        {
        case FloeButtonType::Undefined:
            Debug::log("Undefined", DebugType::Error);
            break;

        case FloeButtonType::Start:
            Debug::log("START");
            m_hovered_start = true;
            break;

        case FloeButtonType::Credits:
            Debug::log("CREDITS");
            m_hovered_credits = true;
            break;

        case FloeButtonType::Exit:
            Debug::log("EXIT");
            m_hovered_exit = true;
            break;
        }
    }
}

void FloeButton::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{
    Component::on_trigger_exit(other);

    std::weak_ptr<LighthouseLight> lighthouse_light = {};
    if (auto const l = other->entity->get_component<LighthouseLight>())
    {
        lighthouse_light = l;

        switch (floe_button_type)
        {
        case FloeButtonType::Undefined:
            Debug::log("Undefined", DebugType::Error);
            break;

        case FloeButtonType::Start:
            Debug::log("START");
            m_hovered_start = false;
            break;

        case FloeButtonType::Credits:
            Debug::log("CREDITS");
            m_hovered_credits = false;
            break;

        case FloeButtonType::Exit:
            Debug::log("EXIT");
            m_hovered_exit = false;
            break;
        }
    }
}

void FloeButton::update()
{
    Component::update();

    if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_LEFT))
    {
        if (m_hovered_start)

            GameController::get_instance()->move_to_next_scene();
    }
}

#if EDITOR
void FloeButton::draw_editor()
{
    Component::draw_editor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Dropdown list
    std::array const floe_button_types = {"Undefined", "Start", "Credits", "Exit"};
    i32 current_item_index = static_cast<i32>(floe_button_type);
    if (ImGui::Combo("Floe Button Type", &current_item_index, floe_button_types.data(), floe_button_types.size()))
    {
        floe_button_type = static_cast<FloeButtonType>(current_item_index);
    }
}
#endif
