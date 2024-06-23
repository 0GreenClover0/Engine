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
#include <imgui_extensions.h>

std::shared_ptr<EndScreen> EndScreen::create()
{
    return std::make_shared<EndScreen>(AK::Badge<EndScreen> {});
}

EndScreen::EndScreen(AK::Badge<EndScreen>)
{
}

void EndScreen::awake()
{
    for (auto star : stars)
    {
        star.lock()->transform->set_local_scale({0.0f, 0.0f, 0.0f});
    }

    update_background();

    set_can_tick(true);
}

void EndScreen::update()
{
    if (!m_is_animation_end)
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

                m_appear_counter = 0.0f;

                if (is_failed)
                {
                    m_is_animation_end = true;
                }
            }
        }
        else
        {
            if (m_shown_stars >= number_of_stars)
            {
                m_is_animation_end = true;
                return;
            }

            if (m_appear_counter < 0.65f)
            {
                m_appear_counter += delta_time * 0.75;
                update_star(m_shown_stars);
            }
            else
            {
                m_appear_counter = 1.0f;

                update_star(m_shown_stars);

                if (m_shown_stars < number_of_stars)
                {
                    m_shown_stars++;
                    m_appear_counter = 0.0f;
                }
            }
        }
    }
}

#if EDITOR
void EndScreen::draw_editor()
{
    Popup::draw_editor();

    if (ImGui::Checkbox("Failed", &is_failed))
    {
        update_background();
    }

    ImGuiEx::draw_ptr("Stars", stars[0]);
    ImGuiEx::draw_ptr("Stars", stars[1]);
    ImGuiEx::draw_ptr("Stars", stars[2]);

    if (ImGui::Button("Save star size"))
    {
        star_scale = glm::vec2(stars[0].lock()->transform->get_local_scale().x, stars[0].lock()->transform->get_local_scale().y);
    }

    unsigned int const min_level = 1;
    unsigned int const max_level = 3;

    ImGui::SliderScalar("Number of stars: ", ImGuiDataType_U32, &number_of_stars, &min_level, &max_level);
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

void EndScreen::update_star(u32 star_number)
{
    stars[star_number].lock()->transform->set_local_scale(
        {easeOutElastic(m_appear_counter) * star_scale.x, easeOutElastic(m_appear_counter) * star_scale.y, 0.0f});
}
