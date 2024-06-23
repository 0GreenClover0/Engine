#include "AK/AK.h"
#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "Model.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#if EDITOR
#include <imgui.h>
#endif
#include "Clock.h"
#include "Credits.h"
#include "FloeButton.h"
#include "GameController.h"
#include <Panel.h>
#include <imgui_extensions.h>

std::shared_ptr<Clock> Clock::create()
{
    auto instance = std::make_shared<Clock>(AK::Badge<Clock> {});

    if (m_instance)
    {
        Debug::log("Instance of clock already exists in the scene.", DebugType::Error);
    }

    m_instance = instance;
    return instance;
}

Clock::Clock(AK::Badge<Clock>)
{
}

std::shared_ptr<Clock> Clock::get_instance()
{
    return m_instance;
}

void Clock::awake()
{
    set_can_tick(false);
}

void Clock::update()
{
}

#if EDITOR
void Clock::draw_editor()
{
    Component::draw_editor();
}
#endif

void Clock::update_visibility()
{
    glm::vec3 position = entity->transform->get_local_position();
    if (GameController::get_instance()->get_level_number() == 0)
    {
        entity->transform->set_local_position({position.x, -5.0f, position.z});
    }
    else
    {
        entity->transform->set_local_position({position.x, 0.85f, position.z});
    }
}
