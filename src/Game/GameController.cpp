#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Input.h"
#include "GameController.h"
#include "Globals.h"
#include "Debug.h"

std::shared_ptr<GameController> GameController::create()
{
    return std::make_shared<GameController>();
}

void GameController::awake()
{
    time = map_time;

    set_can_tick(true);
}

void GameController::update()
{   
    if (time > 0.0f)
    {
        time -= delta_time;
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
}
