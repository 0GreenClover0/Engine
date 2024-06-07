#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>
#include <imgui.h>

#include "AK/AK.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Globals.h"
#include "IceBound.h"
#include "Input.h"
#include "Model.h"
#include "ResourceManager.h"

std::shared_ptr<IceBound> IceBound::create()
{
    return std::make_shared<IceBound>(AK::Badge<IceBound> {});
}

IceBound::IceBound(AK::Badge<IceBound>)
{
}

void IceBound::awake()
{
    set_can_tick(true);
}

void IceBound::update()
{
}

void IceBound::draw_editor()
{
    Component::draw_editor();

    if (entity->get_component<Collider2D>() != nullptr && entity->get_component<Model>() != nullptr)
    {
        bool is_dirty = false;

        std::array const ice_types = {"Rectangle", "Circle"};
        i32 current_item_index = static_cast<i32>(m_type);
        if (ImGui::Combo("Collider Type", &current_item_index, ice_types.data(), ice_types.size()))
        {
            m_type = static_cast<ColliderType2D>(current_item_index);
            if (m_type == ColliderType2D::Rectangle)
            {
                if (m_size > 4)
                {
                    m_size = 4;
                }
            }

            entity->get_component<Collider2D>()->set_collider_type(m_type);

            is_dirty = true;
        }

        u32 constexpr min_size = 1;
        u32 const max_size = m_type == ColliderType2D::Circle ? 7 : 4;

        is_dirty |= ImGui::SliderScalar("Size: ", ImGuiDataType_U32, &m_size, &min_size, &max_size);

        if (is_dirty)
        {
            is_dirty = false;

            if (m_type == ColliderType2D::Circle)
            {
                if (m_size < 1 || m_size > max_size)
                {
                    Debug::log("Error: Wrong ice bound size " + std::to_string(m_size), DebugType::Error);
                    return;
                }

                entity->get_component<Model>()->model_path = "./res/models/iceIslands/c_" + std::to_string(m_size) + ".gltf";

                switch (m_size)
                {
                case 1:
                    entity->get_component<Collider2D>()->set_radius_2d(0.4f);
                    break;
                case 2:
                    entity->get_component<Collider2D>()->set_radius_2d(0.4f);
                    break;
                case 3:
                    entity->get_component<Collider2D>()->set_radius_2d(0.8f);
                    break;
                case 4:
                    entity->get_component<Collider2D>()->set_radius_2d(0.8f);
                    break;
                case 5:
                    entity->get_component<Collider2D>()->set_radius_2d(1.25f);
                    break;
                case 6:
                    entity->get_component<Collider2D>()->set_radius_2d(1.4f);
                    break;
                case 7:
                    entity->get_component<Collider2D>()->set_radius_2d(2.0f);
                    break;
                default:
                    std::unreachable();
                }

                entity->get_component<Model>()->reprepare();
                entity->get_component<Collider2D>()->update_center_and_corners();
            }
            else
            {
                if (m_size < 1 || m_size > max_size)
                {
                    Debug::log("Error: Wrong ice bound size " + std::to_string(m_size), DebugType::Error);
                    return;
                }

                entity->get_component<Model>()->model_path = "./res/models/iceIslands/s_" + std::to_string(m_size) + ".gltf";

                switch (m_size)
                {
                case 1:
                    entity->get_component<Collider2D>()->set_bounds_dimensions_2d(0.4f, 0.4f);
                    break;
                case 2:
                    entity->get_component<Collider2D>()->set_bounds_dimensions_2d(0.6f, 0.6f);
                    break;
                case 3:
                    entity->get_component<Collider2D>()->set_bounds_dimensions_2d(1.4f, 1.3f);
                    break;
                case 4:
                    entity->get_component<Collider2D>()->set_bounds_dimensions_2d(2.0f, 1.9f);
                    break;
                default:
                    std::unreachable();
                }
            }

            entity->get_component<Model>()->reprepare();
            entity->get_component<Collider2D>()->update_center_and_corners();
        }
    }
    else
    {
        if (ImGui::Button("Add components"))
        {
            auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
            auto const standard_material = Material::create(standard_shader);

            entity->add_component(Collider2D::create(glm::vec2(0.4f, 0.4f), false));
            entity->get_component<Collider2D>()->is_trigger = true;
            entity->get_component<Collider2D>()->set_collider_type(ColliderType2D::Rectangle);

            entity->add_component(Model::create("./res/models/iceIslands/s_1.gltf", standard_material));
        }
    }
}
