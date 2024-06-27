#include "LevelController.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Clock.h"
#include "EndScreen.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "Input.h"
#include "Player.h"
#include "SceneSerializer.h"
#include "ScreenText.h"
#include "Ship.h"
#include "ShipSpawner.h"

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
    Clock::get_instance()->update_visibility();
}

void LevelController::update()
{
    if (is_ended)
    {
        return;
    }

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
        if (is_tutorial)
        {
            clock_locked->set_text("--:--");
        }
        else
        {
            clock_locked->set_text(min + ":" + sec);
        }
        clock_locked->color = 0xFFD6856B;
        clock_locked->font_size = 65;
    }
    else
    {
        Debug::log("CLOCK ScreenText is not attached. UI is not working properly.", DebugType::Error);
    }

    if (is_started)
    {
        if (time > 0.0f)
        {
            if (AK::Math::are_nearly_equal(Player::get_instance()->flash_counter, 0.0f))
            {
                if (!is_tutorial && GameController::get_instance()->get_level_number() != 0)
                {
                    time -= delta_time;
                }
            }

            if (!is_tutorial && GameController::get_instance()->get_level_number() != 0)
            {
                if (Player::get_instance()->food >= LevelController::get_instance()->map_food)
                {
                    end_level();
                    return;
                }

                if (entity->get_component<ShipSpawner>()->is_last_chance_activated()
                    && entity->get_component<ShipSpawner>()->get_number_of_food_ships() == 0)
                {
                    end_level();
                    return;
                }
            }
        }
        else
        {
            end_level();
            return;
        }

        float const x = time / map_time * -1.0 + 1.0f;

        if (!is_tutorial)
        {
            ships_limit = glm::ceil(ships_limit_curve.lock()->get_y_at(x));
        }
        ships_speed = ships_speed_curve.lock()->get_y_at(x);

        // For disabling tutorial WASD prompt when player uses WASD.
        if (!m_story_wasd_prompt.expired()
            && (Input::input->get_key_down(GLFW_KEY_W) || Input::input->get_key_down(GLFW_KEY_S) || Input::input->get_key_down(GLFW_KEY_A)
                || Input::input->get_key_down(GLFW_KEY_D)))
        {
            m_story_wasd_prompt.lock()->destroy_immediate();
        }

        // For disabling second space prompt in tutorial when space is used.
        if (!m_story_second_space_prompt.expired() && Input::input->get_key_down(GLFW_KEY_SPACE))
        {
            m_story_second_space_prompt.lock()->destroy_immediate();
        }
    }
    else
    {
        if (GameController::get_instance()->get_level_number() == 0) // We're in menu
        {
            lighthouse.lock()->turn_light(true);
            is_started = true;
            set_exiting_lighthouse(true);

            if (GameController::get_instance()->get_level_number() == 0) // We're in menu
            {
                Player::get_instance()->lighthouse_level = LevelController::get_instance()->maximum_lighthouse_level;
                LevelController::get_instance()->on_lighthouse_upgraded();
            }
        }
        else
        {
            spawn_mouse_prompt_if_needed();
            if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_LEFT))
            {
                if (!is_tutorial)
                {
                    set_exiting_lighthouse(true);
                }

                if (!m_story_mouse_prompt.expired())
                    m_story_mouse_prompt.lock()->destroy_immediate();

                check_tutorial_progress(TutorialProgressAction::LighthouseEnabled);
                lighthouse.lock()->turn_light(true);
                is_started = true;

                if (GameController::get_instance()->get_level_number() == 4)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                }
            }
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
    ImGuiEx::draw_ptr("Lighthouse", lighthouse);
    ImGuiEx::draw_ptr("Customer manager", customer_manager);

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
    ImGui::InputScalar("Maximum lighthouse level: ", ImGuiDataType_U32, &maximum_lighthouse_level);

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
        u32 constexpr min_level = 1;
        u32 constexpr max_level = 3;

        ImGui::SliderScalar("Tutorial Level: ", ImGuiDataType_U32, &tutorial_level, &min_level, &max_level);
        ImGui::Text(("Tutorial Progress: " + std::to_string(tutorial_progress)).c_str());
        ImGui::Text(("Tutorial Spawn Path: " + std::to_string(tutorial_spawn_path)).c_str());
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

void LevelController::set_exiting_lighthouse(bool value)
{
    m_is_exiting_lighthouse_enabled = value;
}

bool LevelController::get_exiting_lighthouse()
{
    return m_is_exiting_lighthouse_enabled;
}

void LevelController::check_tutorial_progress(TutorialProgressAction action)
{
    if (action == TutorialProgressAction::LevelStarted)
    {
        // NOTE: This is happening before level number is increased, so the actual levels are +1.
        if (GameController::get_instance()->get_level_number() == 3)
        {
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {-2.5f, 2.0f}, 90.0f, true);
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {-3.5f, 2.0f}, 90.0f, true);
        }

        if (GameController::get_instance()->get_level_number() == 4)
        {
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {2.0f, 1.1f}, 0.0f, true);
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {2.0f, 2.1f}, 0.0f, true);
        }

        if (GameController::get_instance()->get_level_number() == 5)
        {
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {-3.8f, -4.3f}, 230.0f, true);
            entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {-5.0f, -3.0f}, 230.0f, true);
        }
    }

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
            if (action == TutorialProgressAction::LevelStarted)
            {
                ships_limit = 1;
                entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::FoodSmall, {-6.2f, 0.05f}, 0.0f);
                GameController::get_instance()->dialog_manager.lock()->play_content(0);
                progress_tutorial();
            }
            break;
        case 1:
            if (action == TutorialProgressAction::LighthouseEnabled)
            {
                GameController::get_instance()->dialog_manager.lock()->end_content();
                progress_tutorial();
            }
            break;
        case 2:
            if (action == TutorialProgressAction::ShipEnteredControl)
            {
                progress_tutorial();
                break;
            }
            if (action == TutorialProgressAction::ShipDestroyed)
            {
                progress_tutorial();
                break;
            }
            break;
        case 3:
            if (action == TutorialProgressAction::ShipDestroyed)
            {
                GameController::get_instance()->dialog_manager.lock()->play_content(1);
                progress_tutorial();
            }
            if (action == TutorialProgressAction::ShipEnteredPort)
            {
                GameController::get_instance()->dialog_manager.lock()->play_content(2);
                m_story_space_prompt = SceneSerializer::load_prefab("SpacePrompt");
                m_story_space_prompt.lock()->transform->set_position(m_space_prompt_pos);
                entity->get_component<ShipSpawner>()->set_glow_to_last_ship();
                set_exiting_lighthouse(true);
                progress_tutorial(2);
            }
            break;
        case 4:
            if (action == TutorialProgressAction::ShipEnteredPort)
            {
                GameController::get_instance()->dialog_manager.lock()->play_content(2);
                m_story_space_prompt = SceneSerializer::load_prefab("SpacePrompt");
                m_story_space_prompt.lock()->transform->set_position(m_space_prompt_pos);
                set_exiting_lighthouse(true);
                progress_tutorial();
            }
            break;
        case 5:
            if (action == TutorialProgressAction::KeeperLeftLighthouse)
            {
                GameController::get_instance()->dialog_manager.lock()->end_content();

                if (!m_story_space_prompt.expired())
                    m_story_space_prompt.lock()->destroy_immediate();

                m_story_wasd_prompt = SceneSerializer::load_prefab("WASDPrompt");
                m_story_wasd_prompt.lock()->transform->set_position(m_wasd_prompt_pos);

                progress_tutorial();
            }
            break;
        case 6:
            if (action == TutorialProgressAction::KeeperEnteredPort)
            {
                progress_tutorial();
            }
            break;
        case 7:
            //TODO: PROMPT [SPACE] Take the package
            if (action == TutorialProgressAction::PackageCollected)
            {
                GameController::get_instance()->dialog_manager.lock()->play_content(3);
                entity->get_component<ShipSpawner>()->set_enabled(false);
                progress_tutorial();
            }
            break;
        case 8:
            if (action == TutorialProgressAction::DialogEnded && customer_manager.lock()->get_number_of_customers() == 0)
            {
                end_level();
            }
            break;
        }
        break;
    case 2:
        switch (tutorial_progress)
        {
        case 0:
            if (action == TutorialProgressAction::LevelStarted)
            {
                ships_limit = 1;
                entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {3.5f, 3.1f}, 270.0f);
                GameController::get_instance()->dialog_manager.lock()->flip(true);
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->play_content(4);
                }
                progress_tutorial();
            }
            break;
        case 1:
            if (action == TutorialProgressAction::LighthouseEnabled)
            {
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                }
                set_exiting_lighthouse(true);
                progress_tutorial();
            }
            break;
        case 2:
            if (action == TutorialProgressAction::ShipEnteredControl)
            {
                progress_tutorial();
                break;
            }
            if (action == TutorialProgressAction::ShipDestroyed)
            {
                progress_tutorial();
                break;
            }
            break;
        case 3:
            if (action == TutorialProgressAction::ShipEnteredPort)
            {
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->play_content(5);

                    if (lighthouse.lock()->is_keeper_inside())
                    {
                        m_story_second_space_prompt = SceneSerializer::load_prefab("SpacePrompt");
                        m_story_second_space_prompt.lock()->transform->set_position(m_second_space_prompt_pos);
                    }
                }
                progress_tutorial();
            }
            break;
        case 4:
            if (action == TutorialProgressAction::PackageCollected)
            {
                entity->get_component<ShipSpawner>()->set_enabled(false);
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->play_content(6);
                }
                factories[1].lock()->set_glowing(true);
                progress_tutorial();
            }
            break;
        case 5:
            //TODO: PROMPT [SPACE] Fuel The generator
            if (action == TutorialProgressAction::GeneratorFueled)
            {
                entity->get_component<ShipSpawner>()->set_enabled(true);
                entity->get_component<ShipSpawner>()->pop_event();
                tutorial_spawn_path = 1;
                is_tutorial_dialogs_enabled = true;
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(7);
                }
                factories[1].lock()->set_glowing(false);
                progress_tutorial();
            }
            break;
        case 6:
            //TODO: PROMPT [RMB] Flash!
            if (action == TutorialProgressAction::PackageCollected)
            {
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(10);
                }
                entity->get_component<ShipSpawner>()->set_enabled(false);
                progress_tutorial(3);
                break;
            }
            if (Player::get_instance()->flash == 0 && action == TutorialProgressAction::ShipDestroyed)
            {
                tutorial_spawn_path = 0;
                entity->get_component<ShipSpawner>()->reset_event();
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(8);
                }
                progress_tutorial();
                break;
            }
            if (Player::get_instance()->flash == 0 && action == TutorialProgressAction::ShipInFlashCollider)
            {
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(9);
                }
                progress_tutorial(2);
            }
            break;
        case 7:
            if (action == TutorialProgressAction::DialogEnded)
            {
                is_tutorial_dialogs_enabled = false;
                progress_tutorial(-5);
            }
            break;
        case 8:
            if (Player::get_instance()->flash == 0 && action == TutorialProgressAction::ShipDestroyed)
            {
                tutorial_spawn_path = 0;
                entity->get_component<ShipSpawner>()->reset_event();
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(8);
                }
                progress_tutorial(-1);
                break;
            }
            if (action == TutorialProgressAction::PackageCollected)
            {
                if (is_tutorial_dialogs_enabled)
                {
                    GameController::get_instance()->dialog_manager.lock()->end_content();
                    GameController::get_instance()->dialog_manager.lock()->play_content(10);
                }
                entity->get_component<ShipSpawner>()->set_enabled(false);
                progress_tutorial();
            }
            break;
        case 9:
            if (action == TutorialProgressAction::DialogEnded && customer_manager.lock()->get_number_of_customers() == 0)
            {
                end_level();
            }
            break;
        }
        break;
    case 3:
        switch (tutorial_progress)
        {
        case 0:
            if (action == TutorialProgressAction::LevelStarted)
            {
                ships_limit = 1;
                entity->get_component<ShipSpawner>()->spawn_ship_at_position(ShipType::Tool, {6.8f, 0.0f}, 180.0f);
                GameController::get_instance()->dialog_manager.lock()->flip(false);
                progress_tutorial();
            }
            break;
        case 1:
            if (action == TutorialProgressAction::LighthouseEnabled)
            {
                set_exiting_lighthouse(true);
                progress_tutorial();
            }
            break;
        case 2:
            if (action == TutorialProgressAction::ShipEnteredControl)
            {
                progress_tutorial();
            }
            break;
        case 3:
            if (action == TutorialProgressAction::PackageCollected)
            {
                tutorial_spawn_path = 1;
                entity->get_component<ShipSpawner>()->pop_event();
                entity->get_component<ShipSpawner>()->set_enabled(false);
                GameController::get_instance()->dialog_manager.lock()->play_content(11);
                factories[0].lock()->set_glowing(true);

                progress_tutorial();
            }
            break;
        case 4:
            //TODO: PROMPT [SPACE] Upgrade lighthouse
            if (action == TutorialProgressAction::WorkshopUpgraded)
            {
                GameController::get_instance()->dialog_manager.lock()->end_content();
                GameController::get_instance()->dialog_manager.lock()->play_content(12);
                entity->get_component<ShipSpawner>()->set_enabled(true);
                factories[0].lock()->set_glowing(false);
                progress_tutorial();
            }
            break;
        case 5:
            if (action == TutorialProgressAction::PackageCollected)
            {
                ships_limit = 3;
                entity->get_component<ShipSpawner>()->pop_event();
                GameController::get_instance()->dialog_manager.lock()->end_content();
                GameController::get_instance()->dialog_manager.lock()->play_content(15);
                progress_tutorial();
            }
            break;
        case 6:
            if (action == TutorialProgressAction::PirateDestroyed)
            {
                ships_limit--;
                break;
            }

            if (action == TutorialProgressAction::PackageCollected)
            {
                GameController::get_instance()->dialog_manager.lock()->play_content(13);
                entity->get_component<ShipSpawner>()->set_enabled(false);
                progress_tutorial();
            }
            break;
        case 7:
            if (action == TutorialProgressAction::DialogEnded && customer_manager.lock()->get_number_of_customers() == 0)
            {
                end_level();
            }
            break;
        }
        break;
    }
}

void LevelController::progress_tutorial(i32 step)
{
    tutorial_progress += step;
}

void LevelController::spawn_mouse_prompt_if_needed()
{
    if (!m_story_mouse_prompt.expired())
        return;

    if (!GameController::get_instance()->is_moving_to_next_scene())
    {
        m_story_mouse_prompt = SceneSerializer::load_prefab("MousePrompt");
        m_mouse_prompt_pos = lighthouse.lock()->entity->transform->get_position() + glm::vec3(0.0f, 1.3f, -0.75f);
        m_story_mouse_prompt.lock()->transform->set_position(m_mouse_prompt_pos);
    }
}

void LevelController::destroy_mouse_prompt()
{
    if (!m_story_mouse_prompt.expired())
        m_story_mouse_prompt.lock()->destroy_immediate();
}

void LevelController::end_level()
{
    Debug::log(std::to_string(Player::get_instance()->destroyed_ships));

    is_ended = true;
    auto end_screen = SceneSerializer::load_prefab("EndScreen");
    if (Player::get_instance()->food < map_food)
    {
        end_screen->get_component<EndScreen>()->is_failed = true;
        end_screen->get_component<EndScreen>()->update_background();
    }
    else
    {
        if (is_tutorial)
        {
            end_screen->get_component<EndScreen>()->number_of_stars = 1;

            if (Player::get_instance()->destroyed_ships <= 2)
            {
                end_screen->get_component<EndScreen>()->number_of_stars = 3;
            }
            else if (Player::get_instance()->destroyed_ships <= 3)
            {
                end_screen->get_component<EndScreen>()->number_of_stars = 2;
            }
        }
        else
        {
            end_screen->get_component<EndScreen>()->number_of_stars = 1;

            if (Player::get_instance()->destroyed_ships <= 10)
            {
                end_screen->get_component<EndScreen>()->number_of_stars = 3;
            }
            else if (Player::get_instance()->destroyed_ships <= 15)
            {
                end_screen->get_component<EndScreen>()->number_of_stars = 2;
            }
        }
    }

    if (!is_tutorial)
    {
        if (end_screen->get_component<EndScreen>()->is_failed)
        {
            auto const sound = Sound::play_sound("./res/audio/keeper_messages/lose/" + std::to_string(std::rand() % 3 + 1) + ".wav");
            sound->set_volume(0.65f);
        }
        else
        {
            auto const sound = Sound::play_sound("./res/audio/keeper_messages/win/" + std::to_string(std::rand() % 3 + 1) + ".wav");
            sound->set_volume(0.65f);
        }
    }
}
