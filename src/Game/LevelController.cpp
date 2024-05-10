#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Input.h"
#include "LevelController.h"

#include "ScreenText.h"

#include "Globals.h"
#include "Ship.h"
#include "Player.h"

std::shared_ptr<LevelController> LevelController::create()
{
    auto instance = std::make_shared<LevelController>();

    if (m_instance)
    {
        Debug::log("Instance already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
}

std::shared_ptr<LevelController> LevelController::get_instance()
{
    return m_instance;
}

void LevelController::uninitialize()
{
    Component::uninitialize();

    m_instance = nullptr;
}

void LevelController::awake()
{
    Component::initialize();

    time = map_time;

    ships_limit_curve = entity->add_component<Curve>(Curve::create());
    ships_limit_curve.lock()->custom_name = "Ships limit";
    ships_limit_curve.lock()->add_points({
        { 0.0f, 2.0f }, 
        { 0.1f, 2.0f }, 
        { 0.6f, 6.0f }, 
        { 1.0f, 6.0f } 
        });

    ships_speed_curve = entity->add_component<Curve>(Curve::create());
    ships_speed_curve.lock()->custom_name = "Ships speed";
    ships_speed_curve.lock()->add_points({
        { 0.0f, 0.21f },
        { 0.756f, 0.26f },
        { 1.0f, 0.26f }
        });

    ships_range_curve = entity->add_component<Curve>(Curve::create());
    ships_range_curve.lock()->custom_name = "Ships range";
    ships_range_curve.lock()->add_points({
        { 0.0f, 0.4f },
        { 0.5f, 0.6f },
        { 1.0f, 1.5f }
        });

    ships_turn_curve = entity->add_component<Curve>(Curve::create());
    ships_turn_curve.lock()->custom_name = "Ships turn";
    ships_turn_curve.lock()->add_points({
        { 0.0f, 15.0f },
        { 1.0f, 60.0f }
        });

    ships_additional_speed_curve = entity->add_component<Curve>(Curve::create());
    ships_additional_speed_curve.lock()->custom_name = "Ships additional speed";
    ships_additional_speed_curve.lock()->add_points({
        { 0.0f, 0.001f },
        { 0.2f, 0.0025f },
        { 1.0f, 0.01f }
        });

    pirates_in_control_curve = entity->add_component<Curve>(Curve::create());
    pirates_in_control_curve.lock()->custom_name = "Pirates in control";
    pirates_in_control_curve.lock()->add_points({
        { 0.0f, 0.16f },
        { 1.0f, 1.66f }
        });

    set_can_tick(true);

    on_lighthouse_upgraded();
}

void LevelController::update()
{   
    if (time > 0)
    {
        time-=delta_time;
    }

    float x = (((time / map_time) * -1.0) + 1.0f);

    ships_limit = glm::ceil(ships_limit_curve.lock()->get_y_at(x));
    ships_speed = ships_speed_curve.lock()->get_y_at(x);
}

void LevelController::draw_editor()
{
    static i32 minutes = 0;
    static i32 seconds = 0;
    bool is_changed = false;

    ImGui::Text("Map Time");
    is_changed |= ImGui::InputInt("Minutes: ", &minutes);
    is_changed |= ImGui::InputInt("Seconds: ", &seconds);

    if (is_changed)
    {
        map_time = minutes * 60 + seconds;
    }

    ImGui::Text(("Time: " + std::to_string(time)).c_str());
    ImGui::Text(("Ships Limit: " + std::to_string(ships_limit)).c_str());
    ImGui::Text(("Ships Speed: " + std::to_string(ships_speed)).c_str());
}

void LevelController::on_lighthouse_upgraded() const
{
    float const lighthouse_level_ratio = static_cast<float>(Player::get_instance()->lighthouse_level) / static_cast<float>(maximum_lighthouse_level);
    Player::get_instance()->range = ships_range_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->turn_speed = ships_turn_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->additional_ship_speed = ships_additional_speed_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->pirates_in_control = pirates_in_control_curve.lock()->get_y_at(lighthouse_level_ratio);
}
