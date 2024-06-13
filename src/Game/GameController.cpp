#include <GLFW/glfw3.h>
#include <imgui.h>

#include "AK/Math.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "Input.h"
#include <imgui_extensions.h>

std::shared_ptr<GameController> GameController::create()
{
    auto instance = std::make_shared<GameController>(AK::Badge<GameController> {});

    if (m_instance)
    {
        Debug::log("Instance already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
}

GameController::GameController(AK::Badge<GameController>)
{
}

std::shared_ptr<GameController> GameController::get_instance()
{
    return m_instance;
}

void GameController::uninitialize()
{
    Component::uninitialize();

    m_instance = nullptr;
}

void GameController::awake()
{
    set_can_tick(true);
}

void GameController::update()
{
    if (!m_move_to_next_scene)
    {
        return;
    }

    if (m_move_to_next_scene_counter < 1.0f)
    {
        m_move_to_next_scene_counter += 0.01f;
    }
    else
    {
        m_move_to_next_scene_counter = 0.0f;
        m_move_to_next_scene = false;
    }

    current_scene.lock()->transform->set_local_position(
        glm::vec3(0.0f - ease_in_out_cubic(m_move_to_next_scene_counter) * 18.0f, 0.0f, 0.0f));
    next_scene.lock()->transform->set_local_position(
        glm::vec3(18.0f - ease_in_out_cubic(m_move_to_next_scene_counter) * 18.0f, 0.0f, 0.0f));
}

void GameController::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Scene 1", current_scene);
    ImGuiEx::draw_ptr("Scene 2", next_scene);

    if (ImGui::Button("Move"))
    {
        m_move_to_next_scene = true;
    }
}

float GameController::ease_in_out_cubic(float const x) const
{
    return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}
