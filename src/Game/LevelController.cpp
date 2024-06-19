#include "LevelController.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "Input.h"
#include "Player.h"
#include "ScreenText.h"
#include "Ship.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
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
    player_ref = Player::get_instance();

    if (!player_ref.expired())
    {
        clock_text_ref = player_ref.lock()->clock_text;
    }
    else
    {
        Debug::log("Invalid player reference. No valid instance of Player?", DebugType::Error);
    }

    set_can_tick(true);
    on_lighthouse_upgraded();
}

void LevelController::update()
{
    {
        float const y = Input::input->get_mouse_position().y * playfield_height + playfield_y_shift;
        float const x =
            Input::input->get_mouse_position().x
            * (playfield_width - (playfield_additional_width * (Input::input->get_mouse_position().y + playfield_y_shift + 1.0f) / 2.0f));

        RendererDX11::get_instance_dx11()->inject_mouse_position({x, y});
    }

    if (!clock_text_ref.expired())
    {
        std::string min = "00";
        std::string sec = "00";
        AK::extract_time(time, min, sec);

        auto const clock_locked = clock_text_ref.lock();
        clock_locked->set_text(min + ":" + sec);
        clock_locked->color = 0xFFD6856B;
        clock_locked->font_size = 65;
    }
    else
    {
        Debug::log("CLOCK ScreenText is not attached. UI is not working properly.", DebugType::Error);
    }

    if (is_started)
    {
        if (is_tutorial)
        {
            if (Player::get_instance()->food >= map_food)
            {
                GameController::get_instance()->move_to_next_scene();
            }
        }

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
            check_tutorial_progress(TutorialProgressAction::LighthouseEnabled);
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

    ImGui::Separator();

    ImGui::InputScalar("Starting package", ImGuiDataType_U32, &starting_packages);

    ImGui::Separator();

    ImGui::Checkbox("Tutorial", &is_tutorial);

    if (is_tutorial)
    {
        unsigned int const min_level = 1;
        unsigned int const max_level = 3;

        ImGui::SliderScalar("Tutorial Level: ", ImGuiDataType_U32, &tutorial_level, &min_level, &max_level);
        ImGui::Text(("Tutorial Progress: " + std::to_string(tutorial_progress)).c_str());
    }
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

void LevelController::check_tutorial_progress(TutorialProgressAction action)
{
    if (!is_tutorial)
    {
        return;
    }

    switch (tutorial_level)
    {
    case 1:
        switch (tutorial_progress)
        {
        case 0:
            if (action == TutorialProgressAction::ShipEnteredControl)
            {
                progress_tutorial();
            }
            break;
        case 1:
            if (action == TutorialProgressAction::ShipEnteredPort)
            {
                progress_tutorial();
            }
            break;
        case 2:
            if (action == TutorialProgressAction::KeeperLeavedLighthouse)
            {
                progress_tutorial();
            }
            break;
        case 3:
            if (action == TutorialProgressAction::KeeperEnteredPort)
            {
                progress_tutorial();
            }
            break;
        case 4:
            if (action == TutorialProgressAction::PackageCollected)
            {
                progress_tutorial();
            }
            break;
        }
    case 2:
        switch (tutorial_progress)
        {
        case 0:
            //TODO Move to next progress if ship is in port
            progress_tutorial();
            break;
        case 1:
            //TODO Move to next progress if keeper collect package
            progress_tutorial();
            break;
        case 2:
            //TODO Move to next progress if keeper enter generator range
            progress_tutorial();
            break;
        case 3:
            //TODO Move to next progress if keeper enter lighthouse
            progress_tutorial();
            break;
        case 4:
            //TODO Move to next progress if ship enter flash collider
            //TODO Move to other state if player waste flash
            progress_tutorial();
            break;
        case 5:
            //TODO Move to next progress if player use flash
            progress_tutorial();
            break;
        case 6:
            //TODO Move to next progress if keeper collect package
            progress_tutorial();
            break;
        }
    case 3:
        switch (tutorial_progress)
        {
        case 0:
            //TODO Move to next progress if keeper collect package
            progress_tutorial();
            break;
        case 1:
            //TODO Move to next progress if player upgrade lighthouse
            progress_tutorial();
            break;
        case 2:
            //TODO Move to next progress if keeper collect package
            progress_tutorial();
            break;
        case 3:
            //TODO Move to next progress if keeper collect package
            progress_tutorial();
            break;
        }
    }
}

void LevelController::progress_tutorial(u32 step)
{
    tutorial_progress += step;
}
