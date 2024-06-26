#include "GameController.h"

#include "AK/Math.h"
#include "Clock.h"
#include "DebugInputController.h"
#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "LevelController.h"
#include "Path.h"
#include "Player.h"
#include "SceneSerializer.h"
#include "ShipSpawner.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include <imgui.h>
#include <imgui_extensions.h>
#endif

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

u32 GameController::get_level_number() const
{
    return m_level_number;
}

void GameController::uninitialize()
{
    Component::uninitialize();

    m_instance = nullptr;
}

void GameController::awake()
{
    if (!SceneSerializer::load_prefab("DEBUGINPUTCONTROLLER"))
    {
        auto const entity = Entity::create("DEBUGINPUTCONTROLLER");
        entity->add_component<DebugInputController>(DebugInputController::create());
    }

    std::ranges::reverse(m_levels_order);

    m_levels_backup = m_levels_order;

    auto const path = entity->get_component<Path>();

    if (path != nullptr)
    {
        m_points_backup = path->points;
    }
    else
    {
        Debug::log("No Path component present on GameController entity.", DebugType::Error);
    }

    std::string const level = m_levels_order.back();
    m_levels_order.pop_back();
    current_scene = SceneSerializer::load_prefab(level);

    reset_level();

    Sound::play_sound("./res/audio/ost_loop.wav", true);

    Clock::get_instance()->update_visibility();

    set_can_tick(true);
}

void GameController::update()
{
    if (Input::input->get_key_down(GLFW_KEY_F3))
    {
        if (!m_move_to_next_scene)
        {
            GameController::get_instance()->dialog_manager.lock()->end_content();
            move_to_next_scene();
        }
    }

    if (Input::input->get_key_down(GLFW_KEY_F4))
    {
        SceneSerializer::load_prefab("ThanksScreen");
    }

    if (Input::input->get_key_down(GLFW_KEY_F6))
    {
        restart_level();
    }

    if (!m_move_to_next_scene)
    {
        return;
    }

    if (m_move_to_next_scene_counter < 1.0f)
    {
        m_move_to_next_scene_counter += delta_time * 0.75f;

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

        if (m_level_number == 4)
        {
            dialog_manager.lock()->play_content(14);
        }
    }
}

#if EDITOR
void GameController::draw_editor()
{
    Component::draw_editor();

    if (ImGui::Button("Move"))
    {
        move_to_next_scene();
    }

    ImGuiEx::draw_ptr("Dialog manager", dialog_manager);
}
#endif

bool GameController::is_moving_to_next_scene() const
{
    return m_move_to_next_scene;
}

void GameController::reset_scene()
{
    m_levels_order = m_levels_backup;

    m_level_number = 0;

    next_scene = SceneSerializer::load_prefab(m_levels_order.back());
    m_levels_order.pop_back();

    reset_level();

    auto const path = entity->get_component<Path>();
    m_current_position = path->points[path->points.size() - 1];
    m_next_position = path->points[0];

    glm::vec2 const delta = path->points[path->points.size() - 1] - path->points[0];

    for (auto& point : path->points)
    {
        point -= delta;
    }

    path->points = m_points_backup;

    m_move_to_next_scene = true;
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
    LevelController::get_instance()->lighthouse.lock()->turn_light(false);
    LevelController::get_instance()->destroy_mouse_prompt();
    LevelController::get_instance()->destroy_immediate();

    if (m_levels_order.empty())
    {
        reset_scene();
        return;
    }

    next_scene = SceneSerializer::load_prefab(m_levels_order.back());
    m_levels_order.pop_back();

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
    Clock::get_instance()->update_visibility();

    m_move_to_next_scene = true;

    update_scenes_position();
}

void GameController::reset_level()
{
    Player::get_instance()->reset_player();
    Player::get_instance()->packages = LevelController::get_instance()->starting_packages;

    LevelController::get_instance()->entity->get_component<ShipSpawner>()->get_spawn_paths();
    LevelController::get_instance()->on_lighthouse_upgraded();
    LevelController::get_instance()->factories[1].lock()->turn_off_lights();
    LevelController::get_instance()->set_exiting_lighthouse(false);
    LevelController::get_instance()->lighthouse.lock()->turn_light(false);
    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::LevelStarted);
}

void GameController::restart_level()
{
    std::string scene_name = current_scene.lock()->name;
    current_scene.lock()->destroy_immediate();
    current_scene = SceneSerializer::load_prefab(scene_name);

    Player::get_instance()->reset_player();
    Player::get_instance()->packages = LevelController::get_instance()->starting_packages;

    LevelController::get_instance()->entity->get_component<ShipSpawner>()->get_spawn_paths();
    LevelController::get_instance()->on_lighthouse_upgraded();
    LevelController::get_instance()->factories[1].lock()->turn_off_lights();
    LevelController::get_instance()->set_exiting_lighthouse(false);
    LevelController::get_instance()->lighthouse.lock()->turn_light(false);
    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::LevelStarted);
}
