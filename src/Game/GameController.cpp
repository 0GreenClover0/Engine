#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_extensions.h>

#include "AK/Math.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "Input.h"
#include "LevelController.h"
#include "Path.h"
#include "Player.h"
#include "SceneSerializer.h"
#include "ShipSpawner.h"

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
    std::ranges::reverse(m_levels_order);

    std::string const level = m_levels_order.back();
    m_levels_order.pop_back();
    current_scene = SceneSerializer::load_prefab(level);

    reset_level();

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

        update_scenes_position();
    }
    else
    {
        update_scenes_position();

        current_scene.lock()->destroy_immediate();

        current_scene = next_scene;
        current_scene.lock()->transform->set_local_position({0.0f, 0.0f, 0.0f});

        next_scene = {};

        m_move_to_next_scene_counter = 0.0f;
        m_move_to_next_scene = false;
    }
}

void GameController::draw_editor()
{
    Component::draw_editor();

    if (ImGui::Button("Move"))
    {
        move_to_next_scene();
    }
}

bool GameController::is_moving_to_next_scene() const
{
    return m_move_to_next_scene;
}

float GameController::ease_in_out_cubic(float const x) const
{
    return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

void GameController::update_scenes_position() const
{
    current_scene.lock()->transform->set_local_position(
        glm::vec3(m_current_position.x - ease_in_out_cubic(m_move_to_next_scene_counter) * m_next_position.x, 0.0f,
                  m_current_position.y - ease_in_out_cubic(m_move_to_next_scene_counter) * m_next_position.y));
    next_scene.lock()->transform->set_local_position(
        glm::vec3(m_next_position.x - ease_in_out_cubic(m_move_to_next_scene_counter) * m_next_position.x, 0.0f,
                  m_next_position.y - ease_in_out_cubic(m_move_to_next_scene_counter) * m_next_position.y));
}

void GameController::move_to_next_scene()
{
    LevelController::get_instance()->destroy_immediate();

    next_scene = SceneSerializer::load_prefab(m_levels_order.back());

    reset_level();

    auto const& path = entity->get_component<Path>();
    m_current_position = path->points[m_level_number];
    m_next_position = path->points[m_level_number + 1];

    glm::vec2 const delta = path->points[m_level_number + 1] - path->points[m_level_number];

    for (auto& point : path->points)
    {
        point -= delta;
    }

    m_level_number++;

    m_move_to_next_scene = true;
}

void GameController::reset_level()
{
    Player::get_instance()->reset_player();

    LevelController::get_instance()->entity->get_component<ShipSpawner>()->get_spawn_paths();
    LevelController::get_instance()->on_lighthouse_upgraded();
    LevelController::get_instance()->factories[1].lock()->update_lights();
}
