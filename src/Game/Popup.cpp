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
#include "Popup.h"
#include <Panel.h>
#include <imgui_extensions.h>

std::shared_ptr<Popup> Popup::create()
{
    return std::make_shared<Popup>(AK::Badge<Popup> {});
}

Popup::Popup(AK::Badge<Popup>)
{
}

Popup::Popup() = default;

void Popup::awake()
{
    set_can_tick(true);
}

void Popup::update()
{
    if (!m_is_in_screen)
    {
        if (m_appear_counter < 1.0f)
        {
            m_appear_counter += delta_time * 0.75f;
            update_screen_position();
        }
        else
        {
            m_is_in_screen = true;
            m_appear_counter = 1.0f;

            update_screen_position();
        }
    }
}

#if EDITOR
void Popup::draw_editor()
{
    Component::draw_editor();
}
#endif

void Popup::update_screen_position()
{
    entity->transform->set_local_position({0.0f, (1.0f - easeOutBack(m_appear_counter)) * -2.0f, 0.0f});
}

float Popup::easeOutBack(float x)
{
    float const c1 = 1.70158;
    float const c3 = c1 + 1;

    return 1 + c3 * std::pow(x - 1, 3) + c1 * std::pow(x - 1, 2);
}

float Popup::easeOutElastic(float x)
{
    float const c4 = (2 * 3.14) / 3;

    if (x == 0)
    {
        return 0;
    }
    else if (x == 1)
    {
        return 1;
    }
    else
    {
        return std::pow(2, -10 * x) * std::sin((x * 10 - 0.75) * c4) + 1;
    }
}
