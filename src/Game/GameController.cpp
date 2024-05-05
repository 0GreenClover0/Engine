#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Input.h"
#include "GameController.h"
#include "Globals.h"
#include "ScreenText.h"

std::shared_ptr<GameController> GameController::create()
{
    auto instance = std::make_shared<GameController>();

    if (m_instance)
    {
        Debug::log("Instance already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
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
    Component::initialize();

    m_text = entity->add_component<ScreenText>(ScreenText::create());
    m_text.lock()->color = 0xffffffff;
    std::wstring const content = L"Packages: " + std::to_wstring(packages) + L" Level: " + std::to_wstring(lighthouse_level) + L"\n" +
        L"Flash: " + std::to_wstring(flash);
    m_text.lock()->set_text(content);

    time = map_time;

    set_can_tick(true);
}

void GameController::update()
{   
    if (time > 0.0f)
    {
        time -= delta_time;
    }

    if (!m_text.expired())
    {
        std::wstring const content = L"Packages: " + std::to_wstring(packages) + L" Level: " + std::to_wstring(lighthouse_level) + L"\n" +
                               L"Flash: " + std::to_wstring(flash);
        m_text.lock()->set_text(content);
    }

    if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_RIGHT))
    {
        if (flash > 0 && glm::epsilonEqual(flash_counter, 0.0f, 0.0001f))
        {
            flash_counter = flash_time;
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

void GameController::draw_editor()
{
    static i32 minutes = 0;
    static i32 seconds = 0;

    ImGui::Text("Map Time");
    ImGui::InputInt("Minutes: ", &minutes);
    ImGui::InputInt("Seconds: ", &seconds);

    map_time = minutes * 60 + seconds;

    ImGui::Text(("Time: " + std::to_string(time)).c_str());

    ImGui::Text(("Flesh: " + std::to_string(flash_counter)).c_str());
}
