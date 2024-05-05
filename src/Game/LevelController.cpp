#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Input.h"
#include "LevelController.h"

#include "ScreenText.h"

#include "Globals.h"
#include "Ship.h"

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
}

void LevelController::update()
{   
    if (time > 0)
    {
        time-=delta_time;
    }
}

void LevelController::draw_editor()
{
    static i32 minutes = 0;
    static i32 seconds = 0;

    ImGui::Text("Map Time");
    ImGui::InputInt("Minutes: ", &minutes);
    ImGui::InputInt("Seconds: ", &seconds);

    map_time = minutes * 60 + seconds;

    ImGui::Text(("Time: " + std::to_string(time)).c_str());
}

void LevelController::on_lighthouse_upgraded()
{
    //TODO add getting values from curves
    Ship::on_lighthouse_upgraded(360.0f, 300.0f * 0.005f, 50.0f * 0.005f, 2.0f);
}
