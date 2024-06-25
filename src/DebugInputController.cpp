#include "DebugInputController.h"
#include "Input.h"
#include "SceneSerializer.h"

std::shared_ptr<DebugInputController> DebugInputController::create()
{
    auto instance = std::make_shared<DebugInputController>(AK::Badge<DebugInputController> {});

    if (m_instance)
    {
        Debug::log("Instance of DebugInputController already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
}

DebugInputController::DebugInputController(AK::Badge<DebugInputController>)
{
}

std::shared_ptr<DebugInputController> DebugInputController::get_instance()
{
    return m_instance;
}

void DebugInputController::awake()
{
    set_can_tick(true);
}

void DebugInputController::update()
{
    // Gamma correction
    if (Input::input->get_key_down(GLFW_KEY_MINUS))
    {
        gamma -= 0.05f;
        std::cout << "Gamma: " + std::to_string(gamma) << std::endl;
    }
    if (Input::input->get_key_down(GLFW_KEY_EQUAL))
    {
        gamma += 0.05f;
        std::cout << "Gamma: " + std::to_string(gamma) << std::endl;
    }

    // Exposure
    if (Input::input->get_key_down(GLFW_KEY_LEFT_BRACKET))
    {
        exposure -= 0.05f;
        std::cout << "Gamma: " + std::to_string(exposure) << std::endl;
    }
    if (Input::input->get_key_down(GLFW_KEY_RIGHT_BRACKET))
    {
        exposure += 0.05f;
        std::cout << "Gamma: " + std::to_string(exposure) << std::endl;
    }

    if (Input::input->get_key(GLFW_KEY_LEFT_CONTROL) && Input::input->get_key_down(GLFW_KEY_S))
    {
        SceneSerializer::save_prefab(entity, "DEBUGINPUTCONTROLLER");
        std::cout << "Successfully saved DebugInputController" << std::endl;
    }
}

#if EDITOR
void DebugInputController::draw_editor()
{
    ImGui::Text("Gamma Correction");
    ImGui::SliderFloat("Gamma", &gamma, 0.0f, 3.0f);

    ImGui::Text("Exposure");
    ImGui::SliderFloat("Exposure", &exposure, 0.0f, 3.0f);
}
#endif
