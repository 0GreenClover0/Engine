#include "LighthouseKeeper.h"

#include "AK/AK.h"
#include "Entity.h"
#include "ExampleUIBar.h"
#include "Factory.h"
#include "Globals.h"
#include "LevelController.h"
#include "Lighthouse.h"
#include "Player.h"
#include "Port.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>

#if EDITOR
#include <imgui.h>
#include "imgui_extensions.h"
#endif

std::shared_ptr<LighthouseKeeper> LighthouseKeeper::create()
{
    return std::make_shared<LighthouseKeeper>(AK::Badge<LighthouseKeeper> {});
}

LighthouseKeeper::LighthouseKeeper(AK::Badge<LighthouseKeeper>)
{
}

void LighthouseKeeper::awake()
{
    for (u32 i = 0; i < Player::get_instance()->packages; i++)
    {
        add_package();
    }

    set_can_tick(true);
}

void LighthouseKeeper::update()
{
    i32 horizontal = 0;
    i32 vertical = 0;

    if (Input::input->get_key(GLFW_KEY_D))
    {
        horizontal++;
    }
    if (Input::input->get_key(GLFW_KEY_A))
    {
        horizontal--;
    }
    if (Input::input->get_key(GLFW_KEY_W))
    {
        vertical--;
    }
    if (Input::input->get_key(GLFW_KEY_S))
    {
        vertical++;
    }

    m_speed.x += horizontal * acceleration;
    m_speed.y += vertical * acceleration;

    if (horizontal == 0)
    {
        m_speed.x -= glm::sign(m_speed.x) * deceleration;
    }

    if (vertical == 0)
    {
        m_speed.y -= glm::sign(m_speed.y) * deceleration;
    }

    if (abs(m_speed.x) < deceleration)
    {
        m_speed.x = 0.0f;
    }

    if (abs(m_speed.y) < deceleration)
    {
        m_speed.y = 0.0f;
    }

    if (glm::length(m_speed) > maximum_speed)
    {
        m_speed = glm::normalize(m_speed) * maximum_speed;
    }

    glm::vec3 speed_vector = glm::vec3(m_speed.x, 0.0f, m_speed.y);
    speed_vector *= delta_time;

    entity->transform->set_local_position(entity->transform->get_local_position() + speed_vector);

    m_target_package_tilt_x = 0.0f;
    m_target_package_tilt_z = 5.0f * (glm::length(m_speed) / maximum_speed);

    if (glm::length(m_speed) >= maximum_speed * 0.25f)
    {
        float rotation = atan2(speed_vector.x, speed_vector.z) * (180.0f / 3.14f) - 90.0f;
        float actual_rotation = entity->transform->get_euler_angles().y;

        while (rotation > 180.0f)
            rotation -= 360.0f;
        while (rotation < -180.0f)
            rotation += 360.0f;

        while (actual_rotation > 180.0f)
            actual_rotation -= 360.0f;
        while (actual_rotation < -180.0f)
            actual_rotation += 360.0f;

        float delta = rotation - actual_rotation;
        while (delta > 180.0f)
            delta -= 360.0f;
        while (delta < -180.0f)
            delta += 360.0f;

        float new_rotation = actual_rotation + delta * 0.1f;

        entity->transform->set_euler_angles({0.0f, new_rotation, 0.0f});

        if (std::abs(delta) > 0.5f)
        {
            m_target_package_tilt_x = glm::sign(delta) * 5.0f;
        }
    }

    m_package_tilt_x = std::lerp(m_package_tilt_x, m_target_package_tilt_x, 0.04f);
    m_package_tilt_z = std::lerp(m_package_tilt_z, m_target_package_tilt_z, 0.08f);

    for (auto const& package : packages)
    {
        package.lock()->transform->set_euler_angles({m_package_tilt_x, 0.0f, m_package_tilt_z});
    }

    handle_input();
}

#if EDITOR
void LighthouseKeeper::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::InputFloat("Acceleration", &acceleration);
    ImGuiEx::InputFloat("Deceleration", &deceleration);
    ImGuiEx::InputFloat("Maximum speed", &maximum_speed);

    ImGuiEx::draw_ptr("Lighthouse", lighthouse);
}
#endif

bool LighthouseKeeper::is_inside_port() const
{
    return m_is_inside_port;
}

void LighthouseKeeper::set_is_inside_port(bool const value)
{
    m_is_inside_port = value;
}

glm::vec2 LighthouseKeeper::get_speed() const
{
    return m_speed;
}

void LighthouseKeeper::handle_input()
{
    auto const& factories = LevelController::get_instance()->factories;
    if (factories.size() > 0 && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        std::shared_ptr<Factory> closest_factory = factories[0].lock();
        float closest_distance = distance(AK::convert_3d_to_2d(closest_factory->entity->transform->get_position()),
                                          AK::convert_3d_to_2d(entity->transform->get_position()));

        for (u32 i = 1; i < factories.size(); ++i)
        {
            auto const factory_locked = factories[i].lock();
            float const distance = glm::distance(AK::convert_3d_to_2d(factory_locked->entity->transform->get_position()),
                                                 AK::convert_3d_to_2d(entity->transform->get_position()));

            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_factory = factory_locked;
            }
        }

        if (closest_distance < interact_with_factory_distance)
        {
            if (closest_factory->interact())
            {
                remove_package();
                return;
            }
        }
    }

    if (!port.expired() && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        auto const port_locked = port.lock();
        auto const port_transform = port_locked->entity->transform;

        glm::vec2 const keeper_position = AK::convert_3d_to_2d(entity->transform->get_position());
        glm::vec2 const port_position = AK::convert_3d_to_2d(port_transform->get_position());

        if (is_inside_port())
        {
            bool const has_interacted = port_locked->interact();

            if (has_interacted)
            {
                if (packages.size() < Player::get_instance()->packages)
                {
                    add_package();
                }
                return;
            }
        }
    }

    if (!lighthouse.expired() && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        auto const lighthouse_locked = lighthouse.lock();
        auto const lighthouse_transform = lighthouse_locked->entity->transform;

        glm::vec2 const keeper_position = AK::convert_3d_to_2d(entity->transform->get_position());
        glm::vec2 const lighthouse_position = AK::convert_3d_to_2d(lighthouse_transform->get_position());

        if (distance(keeper_position, lighthouse_position) < lighthouse_locked->enterable_distance)
        {
            lighthouse_locked->enter();
            entity->destroy_immediate();
            return;
        }
    }
}

void LighthouseKeeper::add_package()
{
    auto const& package = Entity::create("Package");

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    package->add_component(Model::create("./res/models/package/package.gltf", standard_material));
    if (packages.size() > 0)
    {
        package->transform->set_parent(packages.back().lock()->transform);
        float x = package->transform->parent.lock()->get_local_position().x;
        float z = package->transform->parent.lock()->get_local_position().z;
        package->transform->set_local_position(glm::vec3(glm::linearRand(-0.015f, 0.015f) - x, 0.13f, glm::linearRand(-0.02f, 0.02f) - z));
    }
    else
    {
        package->transform->set_parent(entity->transform);
    }

    packages.emplace_back(package);
}

void LighthouseKeeper::remove_package()
{
    if (packages.size() > 0)
    {
        packages.back().lock()->destroy_immediate();
        packages.pop_back();
    }
    else
    {
        Debug::log("Game wants to remove package but there is no package to remove!", DebugType::Warning);
    }
}
