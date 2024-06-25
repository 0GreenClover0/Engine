#include "FloeButton.h"

#include "Collider2D.h"
#include "CommonEntities.h"
#include "Entity.h"
#include "ExampleDynamicText.h"
#include "Game/GameController.h"
#include "Game/LevelController.h"
#include "Game/LighthouseLight.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"

bool FloeButton::are_credits_open = false;

std::shared_ptr<FloeButton> FloeButton::create()
{
    auto floe = std::make_shared<FloeButton>(AK::Badge<FloeButton> {});
    return floe;
}

FloeButton::FloeButton(AK::Badge<FloeButton>)
{
}

void FloeButton::awake()
{
    are_credits_open = false;

    set_can_tick(true);
}

void FloeButton::update()
{
    Component::update();

    if (Input::input->get_key_down(GLFW_MOUSE_BUTTON_LEFT) && are_credits_open == false)
    {
        if (m_hovered && floe_button_type == FloeButtonType::Start)
        {
            if (!GameController::get_instance()->is_moving_to_next_scene())
            {
                GameController::get_instance()->move_to_next_scene();
            }
        }

        if (m_hovered && floe_button_type == FloeButtonType::Credits)
        {
            SceneSerializer::load_prefab("CreditScreen");
            FloeButton::are_credits_open = true;
        }

        if (m_hovered && floe_button_type == FloeButtonType::Exit)
        {
            Engine::should_exit = true;
        }
    }

    if (m_hovered && LevelController::get_instance()->lighthouse.lock()->light.lock()->enabled())
    {
        glm::vec3 position = entity->transform->get_local_position();

        float y = std::lerp(position.y, 0.022f, 0.1f);

        entity->transform->set_local_position({position.x, y, position.z});
    }
    else
    {
        glm::vec3 position = entity->transform->get_local_position();

        float y = std::lerp(position.y, -0.118f, 0.1f);

        entity->transform->set_local_position({position.x, y, position.z});
    }
}

void FloeButton::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    std::weak_ptr<LighthouseLight> lighthouse_light = {};
    if (auto const l = other->entity->get_component<LighthouseLight>())
    {
        lighthouse_light = l;
        m_hovered = true;
    }
}

void FloeButton::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{
    Component::on_trigger_exit(other);

    std::weak_ptr<LighthouseLight> lighthouse_light = {};
    if (auto const l = other->entity->get_component<LighthouseLight>())
    {
        lighthouse_light = l;
        m_hovered = false;
    }
}

#if EDITOR
void FloeButton::draw_editor()
{
    Component::draw_editor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Dropdown list
    std::array const floe_button_types = {"Undefined", "Start", "Credits", "Exit"};
    i32 current_item_index = static_cast<i32>(floe_button_type);
    if (ImGui::Combo("Floe Button Type", &current_item_index, floe_button_types.data(), floe_button_types.size()))
    {
        floe_button_type = static_cast<FloeButtonType>(current_item_index);
    }

    if (ImGui::Button("Add cube"))
    {
        auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
        auto const standard_material = Material::create(standard_shader);

        CommonEntities::create_cube("kuba", "./res/textures/color.jpg", standard_material);
    }
}
#endif
