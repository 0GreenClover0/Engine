#include "EndScreen.h"

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
#include <Panel.h>

std::shared_ptr<EndScreen> EndScreen::create()
{
    return std::make_shared<EndScreen>(AK::Badge<EndScreen> {});
}

std::shared_ptr<EndScreen> EndScreen::create(bool is_failed)
{
    auto end_screen = std::make_shared<EndScreen>(AK::Badge<EndScreen> {});

    end_screen->is_failed = is_failed;

    return end_screen;
}

EndScreen::EndScreen(AK::Badge<EndScreen>)
{
}

void EndScreen::awake()
{
    update_background();

    set_can_tick(true);
}

void EndScreen::update()
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
            m_is_in_screen = false;
            m_appear_counter = 1.0f;

            update_screen_position();
        }
    }
}

#if EDITOR
void EndScreen::draw_editor()
{
    Component::draw_editor();

    if (ImGui::Checkbox("Failed", &is_failed))
    {
        update_background();
    }
}
#endif

void EndScreen::update_background()
{
    if (is_failed)
    {
        entity->get_component<Panel>()->background_path = m_failed_background_path;
    }
    else
    {
        entity->get_component<Panel>()->background_path = m_win_background_path;
    }

    entity->get_component<Panel>()->reprepare();
}

void EndScreen::update_screen_position()
{
    entity->transform->set_local_position({0.0f, (1.0f - easeOutBack(m_appear_counter)) * -2.0f, 0.0f});
}

float EndScreen::easeOutBack(float x)
{
    float const c1 = 1.70158;
    float const c3 = c1 + 1;

    return 1 + c3 * std::pow(x - 1, 3) + c1 * std::pow(x - 1, 2);
}
