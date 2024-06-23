#include "Popup.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Entity.h"
#include "Globals.h"
#include "Input.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

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

            if (m_is_hiding)
            {
                entity->destroy_immediate();
            }
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
    if (m_is_hiding)
    {
        entity->transform->set_local_position({0.0f, (ease_out_back(m_appear_counter)) * -2.0f, 0.0f});
    }
    else
    {
        entity->transform->set_local_position({0.0f, (1.0f - ease_out_back(m_appear_counter)) * -2.0f, 0.0f});
    }
}

void Popup::hide()
{
    m_is_hiding = true;
    m_is_in_screen = false;

    m_appear_counter = 0.0f;
}

float Popup::ease_out_back(float const x)
{
    float constexpr c1 = 1.70158f;
    float constexpr c3 = c1 + 1.0f;

    return 1.0f + c3 * std::powf(x - 1.0f, 3.0f) + c1 * std::powf(x - 1.0f, 2.0f);
}

float Popup::ease_out_elastic(float const x)
{
    float constexpr c4 = (2.0f * 3.14f) / 3.0f;

    if (AK::Math::are_nearly_equal(x, 0.0f))
    {
        return 0.0f;
    }

    if (AK::Math::are_nearly_equal(x, 1.0f))
    {
        return 1.0f;
    }

    return std::powf(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * c4) + 1.0f;
}
