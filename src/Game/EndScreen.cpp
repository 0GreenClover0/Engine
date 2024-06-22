#include "EndScreen.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Entity.h"
#include "Globals.h"
#include "Input.h"
#include "Model.h"
#include "Panel.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

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
    Component::draw_editor();

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

    u32 constexpr min_level = 1;
    u32 constexpr max_level = 3;

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

void EndScreen::update_screen_position() const
{
    entity->transform->set_local_position({0.0f, (1.0f - ease_out_back(m_appear_counter)) * -2.0f, 0.0f});
}

void EndScreen::update_star(u32 const star_number) const
{
    stars[star_number].lock()->transform->set_local_scale(
        {ease_out_elastic(m_appear_counter) * star_scale.x, ease_out_elastic(m_appear_counter) * star_scale.y, 0.0f});
}

float EndScreen::ease_out_back(float const x)
{
    float constexpr c1 = 1.70158f;
    float constexpr c3 = c1 + 1.0f;

    return 1.0f + c3 * std::powf(x - 1.0f, 3) + c1 * std::powf(x - 1.0f, 2.0f);
}

float EndScreen::ease_out_elastic(float const x) const
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
