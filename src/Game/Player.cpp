#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "LevelController.h"
#include "Player.h"
#include "ScreenText.h"

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
    m_text = entity->add_component<ScreenText>(ScreenText::create());
    m_text.lock()->color = 0xffffffff;
    std::wstring const content = L"Packages: " + std::to_wstring(packages) + L" Level: " + std::to_wstring(lighthouse_level) + L"\n"
                               + L"Flash: " + std::to_wstring(flash);
    m_text.lock()->set_text(content);

    set_can_tick(true);
}

void Player::update()
{
    if (!m_text.expired())
    {
        std::wstring const content = L"Packages: " + std::to_wstring(packages) + L" Level: " + std::to_wstring(lighthouse_level) + L"\n"
                                   + L"Flash: " + std::to_wstring(flash);
        m_text.lock()->set_text(content);
    }

    if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (flash > 0 && glm::epsilonEqual(flash_counter, 0.0f, 0.0001f))
        {
            flash -= 1;
            flash_counter = m_flash_time;
        }
    }

    if (flash_counter > 0.0f)
    {
        flash_counter -= delta_time;
    }
    else
    {
        flash_counter = 0.0f;
    }
}

void Player::draw_editor()
{
    Component::draw_editor();

    if (Engine::is_game_running())
    {
        if (ImGui::SliderInt("Lighthouse Level", &lighthouse_level, 0, LevelController::get_instance()->maximum_lighthouse_level))
        {
            LevelController::get_instance()->on_lighthouse_upgraded();
        }
    }
    ImGui::Text(("Ships Range: " + std::to_string(Player::get_instance()->range)).c_str());
    ImGui::Text(("Ships Turn: " + std::to_string(Player::get_instance()->turn_speed)).c_str());
    ImGui::Text(("Ships Additional Speed: " + std::to_string(Player::get_instance()->additional_ship_speed)).c_str());
    ImGui::Text(("Pirates In Control: " + std::to_string(Player::get_instance()->pirates_in_control)).c_str());
}
