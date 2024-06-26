#include "EndScreen.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Clock.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "Input.h"
#include "LevelController.h"
#include "Model.h"
#include "Panel.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"

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

    glfwSetInputMode(Engine::window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    update_background();

    LevelController::get_instance()->lighthouse.lock()->is_entering_lighthouse_allowed = false;
    Clock::get_instance()->update_visibility(true);

    set_can_tick(true);
}

void EndScreen::on_enabled()
{
    if (!next_level_button.expired())
    {
        next_level_button.lock()->on_unclicked.attach(&EndScreen::next_level, shared_from_this());
    }

    if (!restart_button.expired())
    {
        restart_button.lock()->on_unclicked.attach(&EndScreen::restart, shared_from_this());
    }

    if (!menu_button.expired())
    {
        menu_button.lock()->on_unclicked.attach(&EndScreen::menu, shared_from_this());
    }
}

void EndScreen::on_disabled()
{
    if (!next_level_button.expired())
    {
        next_level_button.lock()->on_unclicked.detach(shared_from_this());
    }

    if (!restart_button.expired())
    {
        restart_button.lock()->on_unclicked.detach(shared_from_this());
    }

    if (!menu_button.expired())
    {
        // FIXME: For some reason this makes unloading the scene unhappy.
        //menu_button.lock()->on_unclicked.detach(shared_from_this());
    }
}

void EndScreen::update()
{
    if (!m_is_animation_end || m_is_hiding)
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

                if (m_is_hiding)
                {
                    entity->destroy_immediate();
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

    if (Input::input->get_key_down(GLFW_KEY_F4))
    {
        hide();

        // Thanks for playing screen is presented, we don't want to disable the cursor
        glfwSetInputMode(Engine::window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

    u32 constexpr min_level = 1;
    u32 constexpr max_level = 3;

    ImGui::SliderScalar("Number of stars: ", ImGuiDataType_U32, &number_of_stars, &min_level, &max_level);

    ImGuiEx::draw_ptr("Next level button", next_level_button);
    ImGuiEx::draw_ptr("Restart button", restart_button);
    ImGuiEx::draw_ptr("Menu button", menu_button);
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

void EndScreen::update_star(u32 const star_number)
{
    stars[star_number].lock()->transform->set_local_scale(
        {ease_out_elastic(m_appear_counter) * star_scale.x, ease_out_elastic(m_appear_counter) * star_scale.y, 0.0f});
}

void EndScreen::next_level()
{
    GameController::get_instance()->dialog_manager.lock()->end_content();
    GameController::get_instance()->move_to_next_scene();
    hide();
}

void EndScreen::restart()
{
    GameController::get_instance()->restart_level();
    hide();
}

void EndScreen::menu()
{
    Engine::set_game_running(false);
    Engine::set_game_running(true);
}

void EndScreen::hide()
{
    glfwSetInputMode(Engine::window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    m_is_hiding = true;
    m_is_in_screen = false;

    m_appear_counter = 0.0f;
}
