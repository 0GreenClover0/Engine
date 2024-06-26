#include "LighthouseKeeper.h"

#include "AK/AK.h"
#include "Camera.h"
#include "Entity.h"
#include "ExampleUIBar.h"
#include "Factory.h"
#include "Floater.h"
#include "GameController.h"
#include "Globals.h"
#include "IceBound.h"
#include "LevelController.h"
#include "Lighthouse.h"
#include "Player.h"
#include "Port.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
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
    m_engine_sound = Sound::play_sound_at_location("./res/audio/poduszkowiec.wav", entity->transform->get_position(),
                                                   Camera::get_main_camera()->get_position(), false);
    m_engine_sound->set_volume(2.0f);
}

void LighthouseKeeper::update()
{
    if (m_engine_sound->has_finished())
    {
        m_engine_sound = Sound::play_sound_at_location("./res/audio/poduszkowiec.wav", entity->transform->get_position(),
                                                       Camera::get_main_camera()->get_position(), false);
        m_engine_sound->set_volume(2.0f);
    }
    m_engine_sound->entity->transform->set_position(entity->transform->get_position());
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
    ImGuiEx::draw_ptr("Keeper dust", keeper_dust);
    ImGuiEx::draw_ptr("Keeper splash", keeper_splash);
}
#endif

void LighthouseKeeper::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    if (other->entity->get_component<IceBound>())
    {
        entity->get_component<Floater>()->set_enabled(false);
        auto const position = entity->transform->get_position();
        entity->transform->set_position(glm::vec3(position.x, 0.07f, position.z));

        if (!keeper_dust.expired())
        {
            keeper_dust.lock()->max_spawn_count = 15;
            keeper_dust.lock()->min_spawn_count = 15;
        }
        if (!keeper_splash.expired())
        {
            keeper_splash.lock()->min_spawn_count = 0;
            keeper_splash.lock()->max_spawn_count = 0;
        }
    }
}
void LighthouseKeeper::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{
    if (other->entity->get_component<IceBound>())
    {
        entity->get_component<Floater>()->set_enabled(true);

        if (!keeper_dust.expired())
        {
            keeper_dust.lock()->min_spawn_count = 0;
            keeper_dust.lock()->max_spawn_count = 0;
        }

        if (!keeper_splash.expired())
        {
            keeper_splash.lock()->max_spawn_count = 15;
            keeper_splash.lock()->min_spawn_count = 15;
        }
    }
}

void LighthouseKeeper::on_destroyed()
{
    Component::on_destroyed();

    hide_interaction_prompt(WorldPromptType::Factory);
    hide_interaction_prompt(WorldPromptType::Port);
    hide_interaction_prompt(WorldPromptType::Lighthouse);
    m_engine_sound->stop_with_fade(300);
}

bool LighthouseKeeper::is_inside_port() const
{
    return m_is_inside_port;
}

void LighthouseKeeper::set_is_inside_port(bool const value)
{
    m_is_inside_port = value;
}

void LighthouseKeeper::show_interaction_prompt(glm::vec3 const& position, WorldPromptType type)
{
    switch (type)
    {
    case WorldPromptType::Factory:
        if (m_factory_prompt.expired())
        {
            m_factory_prompt = SceneSerializer::load_prefab("SpacePrompt");
            m_factory_prompt.lock()->transform->set_position({position.x, 2.0f, position.z});
        }
        break;

    case WorldPromptType::Lighthouse:
        if (m_lighthouse_prompt.expired())
        {
            m_lighthouse_prompt = SceneSerializer::load_prefab("SpacePrompt");
            m_lighthouse_prompt.lock()->transform->set_position({position.x, 2.0f, position.z});
        }
        break;

    case WorldPromptType::Port:
        if (m_port_prompt.expired())
        {
            m_port_prompt = SceneSerializer::load_prefab("SpacePrompt");
            m_port_prompt.lock()->transform->set_position({position.x, 1.0f, position.z});
        }
        break;
    }
}

void LighthouseKeeper::hide_interaction_prompt(WorldPromptType type)
{
    switch (type)
    {
    case WorldPromptType::Factory:
        if (!m_factory_prompt.expired())
        {
            m_factory_prompt.lock()->destroy_immediate();
            m_factory_prompt.reset();
        }
        break;

    case WorldPromptType::Lighthouse:
        if (!m_lighthouse_prompt.expired())
        {
            m_lighthouse_prompt.lock()->destroy_immediate();
            m_lighthouse_prompt.reset();
        }
        break;

    case WorldPromptType::Port:
        if (!m_port_prompt.expired())
        {
            m_port_prompt.lock()->destroy_immediate();
            m_port_prompt.reset();
        }
        break;
    }
}

glm::vec2 LighthouseKeeper::get_speed() const
{
    return m_speed;
}

void LighthouseKeeper::handle_input()
{
    auto const& factories = LevelController::get_instance()->factories;
    if (factories.size() > 0)
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

        if (closest_distance < interact_with_factory_distance && Player::get_instance()->packages > 0)
        {
            show_interaction_prompt(closest_factory->entity->transform->get_position(), WorldPromptType::Factory);

            if (Input::input->get_key_down(GLFW_KEY_SPACE))
            {
                if (closest_factory->interact())
                {
                    remove_package();
                    if (closest_factory->type == FactoryType::Generator)
                    {
                        auto const generator_sound = Sound::play_sound_at_location(
                            "./res/audio/generator.wav", entity->transform->get_position(), Camera::get_main_camera()->get_position());
                        generator_sound->set_volume(50.0f);
                        generator_sound->stop_with_fade(1500);
                    }
                    else if (closest_factory->type == FactoryType::Workshop)
                    {
                        auto const workshop_sound = Sound::play_sound_at_location(
                            "./res/audio/warsztat.wav", entity->transform->get_position(), Camera::get_main_camera()->get_position());
                        workshop_sound->set_volume(30.0f);
                        workshop_sound->stop_with_fade(1500);
                    }
                    return;
                }
            }
        }
        else
        {
            hide_interaction_prompt(WorldPromptType::Factory);
        }
    }

    if (!port.expired() && is_inside_port() && !port.lock()->get_ships_inside().empty())
    {
        auto const port_locked = port.lock();
        auto const port_transform = port_locked->entity->transform;

        show_interaction_prompt(port_transform->get_position(), WorldPromptType::Port);

        if (Input::input->get_key_down(GLFW_KEY_SPACE))
        {
            bool const has_interacted = port_locked->interact();

            if (has_interacted)
            {
                hide_interaction_prompt(WorldPromptType::Port);

                auto const pickup_sound =
                    Sound::play_sound_at_location("./res/audio/pickup/paczka" + std::to_string(std::rand() % 2 + 1) + ".wav",
                                                  entity->transform->get_position(), Camera::get_main_camera()->get_position());
                pickup_sound->set_volume(15.0f);

                if (packages.size() < Player::get_instance()->packages)
                {
                    add_package();
                }

                return;
            }
        }
    }
    else if (!port.expired() && !is_inside_port())
    {
        hide_interaction_prompt(WorldPromptType::Port);
    }

    if (!lighthouse.expired())
    {
        auto const lighthouse_locked = lighthouse.lock();
        auto const lighthouse_transform = lighthouse_locked->entity->transform;

        glm::vec2 const keeper_position = AK::convert_3d_to_2d(entity->transform->get_position());
        glm::vec2 const lighthouse_position = AK::convert_3d_to_2d(lighthouse_transform->get_position());

        if (distance(keeper_position, lighthouse_position) < lighthouse_locked->enterable_distance
            && GameController::get_instance()->get_level_number() != 1)
        {
            show_interaction_prompt(lighthouse_transform->get_position(), WorldPromptType::Lighthouse);

            if (Input::input->get_key_down(GLFW_KEY_SPACE) && lighthouse.lock()->is_entering_lighthouse_allowed)
            {
                lighthouse_locked->enter();
                entity->destroy_immediate();
                m_engine_sound->stop_with_fade(1000);
                hide_interaction_prompt(WorldPromptType::Lighthouse);
                return;
            }
        }
        else
        {
            hide_interaction_prompt(WorldPromptType::Lighthouse);
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
