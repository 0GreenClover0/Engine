#include "Player.h"

#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "LevelController.h"
#include "ScreenText.h"
#include "ShipSpawner.h"

#include <GLFW/glfw3.h>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<Player> Player::create()
{
    auto instance = std::make_shared<Player>(AK::Badge<Player> {});

    if (m_instance)
    {
        Debug::log("Instance already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
}

void Player::uninitialize()
{
    Component::uninitialize();

    m_instance = nullptr;
}

void Player::reset_player()
{
    food = 0;
    flash = 0;
    flash_counter = 0.0f;
    packages = 2;
    lighthouse_level = 0;

    range = 0.0f;
    additional_ship_speed = 0.0f;
    turn_speed = 0.0f;
    pirates_in_control = 0.0f;
}

void Player::upgrade_lighthouse()
{
    lighthouse_level++;
    LevelController::get_instance()->on_lighthouse_upgraded();
}

std::shared_ptr<Player> Player::get_instance()
{
    return m_instance;
}

Player::Player(AK::Badge<Player>)
{
}

void Player::awake()
{
    set_can_tick(true);
}

void Player::update()
{
    if (LevelController::get_instance() != nullptr && !LevelController::get_instance()->entity->get_component<ShipSpawner>()->is_any_ship_controlled())
    {
        RendererDX11::get_instance_dx11()->inject_light_range(range);
    }
    else
    {
        RendererDX11::get_instance_dx11()->inject_light_range(0.0f);
    }

    if (!packages_text.expired())
    {
        std::string const content = std::to_string(packages);
        packages_text.lock()->color = 0xfff4ecf8;
        packages_text.lock()->set_text(content);
    }
    else
    {
        Debug::log("PACKAGES ScreenText is not attached. UI is not working properly.", DebugType::Error);
    }

    if (!flashes_text.expired())
    {
        std::string const content = std::to_string(flash);
        flashes_text.lock()->set_text(content);
        flashes_text.lock()->color = 0xfff4ecf8;
    }
    else
    {
        Debug::log("FLASHES ScreenText is not attached. UI is not working properly.", DebugType::Error);
    }

    if (!level_text.expired() && LevelController::get_instance() != nullptr)
    {
        std::string const content =
            std::to_string(lighthouse_level) + "/" + std::to_string(LevelController::get_instance()->maximum_lighthouse_level);

        level_text.lock()->color = 0xfff4ecf8;
        level_text.lock()->set_text(content);
    }
    else
    {
        Debug::log("LEVELS ScreenText is not attached. UI is not working properly.", DebugType::Error);
    }

    if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (flash > 0 && glm::epsilonEqual(flash_counter, 0.0f, 0.0001f))
        {
            flash -= 1;
            flash_counter = m_flash_time;

            for (size_t i = 0; i < LevelController::get_instance()->factories.size(); i++)
            {
                if (LevelController::get_instance()->factories[i].lock()->type == FactoryType::Generator)
                {
                    LevelController::get_instance()->factories[i].lock()->update_lights();
                }
            }

            LevelController::get_instance()->entity->get_component<ShipSpawner>()->burn_out_all_ships(true);
        }
    }

    if (flash_counter > 0.0f)
    {
        flash_counter -= delta_time;
    }
    else
    {
        flash_counter = 0.0f;
        LevelController::get_instance()->entity->get_component<ShipSpawner>()->burn_out_all_ships(false);
    }
}

#if EDITOR
void Player::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Packages UI Text", packages_text);
    ImGuiEx::draw_ptr("Flashes UI Text", flashes_text);
    ImGuiEx::draw_ptr("Level UI Text", level_text);
    ImGuiEx::draw_ptr("Clock UI Text", clock_text);

    if (Engine::is_game_running())
    {

        if (LevelController::get_instance() != nullptr
            && ImGui::SliderInt("Lighthouse Level", &lighthouse_level, 0, LevelController::get_instance()->maximum_lighthouse_level))
        {
            LevelController::get_instance()->on_lighthouse_upgraded();
        }

        ImGui::Text(("Ships Range: " + std::to_string(Player::get_instance()->range)).c_str());
        ImGui::Text(("Ships Turn: " + std::to_string(Player::get_instance()->turn_speed)).c_str());
        ImGui::Text(("Ships Additional Speed: " + std::to_string(Player::get_instance()->additional_ship_speed)).c_str());
        ImGui::Text(("Pirates In Control: " + std::to_string(Player::get_instance()->pirates_in_control)).c_str());
    }
}
#endif
