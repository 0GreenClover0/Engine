#include "LevelController.h"

#include "AK/Math.h"
#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "Player.h"
#include "ScreenText.h"
#include "Ship.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include <imgui.h>
#include "imgui_extensions.h"
#endif

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

    set_can_tick(true);

    on_lighthouse_upgraded();
}

void LevelController::update()
{
    if (is_started)
    {
        if (time > 0.0f)
        {
            if (AK::Math::are_nearly_equal(Player::get_instance()->flash_counter, 0.0f))
            {
                time -= delta_time;
            }
        }
        else
        {
            Debug::log(std::to_string(Player::get_instance()->food) + " / " + std::to_string(map_food));
            Engine::set_game_running(false);
            return;
        }

        float const x = time / map_time * -1.0 + 1.0f;

        ships_limit = glm::ceil(ships_limit_curve.lock()->get_y_at(x));
        ships_speed = ships_speed_curve.lock()->get_y_at(x);
    }
    else
    {
        // TODO: Change RMB to LMB in final game
        if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_RIGHT))
        {
            is_started = true;
        }
    }
}

#if EDITOR
void LevelController::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Workshop", factories[0]);
    ImGuiEx::draw_ptr("Generator", factories[1]);
    ImGuiEx::draw_ptr("Port", port);

    ImGui::Separator();

    static i32 minutes = 0;
    static i32 seconds = 0;
    bool is_time_changed = false;

    ImGui::Text("Map Time");
    is_time_changed |= ImGui::InputInt("Minutes: ", &minutes);
    is_time_changed |= ImGui::InputInt("Seconds: ", &seconds);

    if (is_time_changed)
    {
        map_time = minutes * 60 + seconds;
    }
    ImGui::Text(("Time: " + std::to_string(time)).c_str());

    ImGui::Separator();

    ImGui::Text("Map Food");
    ImGui::InputScalar("Food: ", ImGuiDataType_U32, &map_food);

    ImGui::Text(("Food: " + std::to_string(Player::get_instance()->food) + " / " + std::to_string(map_food)).c_str());

    ImGui::Separator();

    ImGui::Text(("Ships Limit: " + std::to_string(ships_limit)).c_str());
    ImGui::Text(("Ships Speed: " + std::to_string(ships_speed)).c_str());

    ImGui::Separator();

    ImGui::InputFloat("Playfield width: ", &playfield_width);
    ImGui::InputFloat("Playfield additional width: ", &playfield_additional_width);
    ImGui::InputFloat("Playfield height: ", &playfield_height);
    ImGui::InputFloat("Playfield Y shift: ", &playfield_y_shift);
}
#endif

void LevelController::on_lighthouse_upgraded() const
{
    float const lighthouse_level_ratio =
        static_cast<float>(Player::get_instance()->lighthouse_level) / static_cast<float>(maximum_lighthouse_level);
    Player::get_instance()->range = ships_range_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->turn_speed = ships_turn_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->additional_ship_speed = ships_additional_speed_curve.lock()->get_y_at(lighthouse_level_ratio);
    Player::get_instance()->pirates_in_control = pirates_in_control_curve.lock()->get_y_at(lighthouse_level_ratio);
}
