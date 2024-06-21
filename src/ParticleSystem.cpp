#include "ParticleSystem.h"

#include "AK/AK.h"
#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Particle.h"

#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.inl>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

std::shared_ptr<ParticleSystem> ParticleSystem::create()
{
    auto particle_system = std::make_shared<ParticleSystem>(AK::Badge<ParticleSystem> {});
    return particle_system;
}

ParticleSystem::ParticleSystem(AK::Badge<ParticleSystem>)
{
}

void ParticleSystem::awake()
{
    set_can_tick(true);
}

#if EDITOR
void ParticleSystem::draw_editor()
{
    Component::draw_editor();

    ImGui::InputText("Sprite", &sprite_path);
    ImGui::ColorEdit4("Start color 1", value_ptr(start_color_1));
    ImGui::ColorEdit4("End color 1", value_ptr(end_color_1));
    ImGuiEx::InputFloat("Lifetime 1", &lifetime_1);
    ImGuiEx::InputFloat("Lifetime 2", &lifetime_2);
    ImGui::DragFloatRange2("Spawn interval", &min_spawn_interval, &max_spawn_interval, 0.1f, 0.0f, FLT_MAX);
    ImGui::InputFloat3("Start velocity 1", glm::value_ptr(start_velocity_1));
    ImGui::InputFloat3("Start velocity 2", glm::value_ptr(start_velocity_2));
    ImGui::DragFloatRange2("Size", &min_particle_size, &max_particle_size, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragFloat("Emitter size", &emitter_bounds, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragIntRange2("Spawn count", &min_spawn_count, &max_spawn_count, 1, 0, INT_MAX);
    ImGui::Checkbox("Simulate in world space", &m_simulate_in_world_space);
}
#endif

void ParticleSystem::update()
{
    update_system();
}

void ParticleSystem::update_system()
{
    if (m_spawn_data_vector.empty())
    {
        spawn_calculations();
    }
    else
    {
        //  TODO: Modes in shader/cbuffer: override/multiply color, adjustable alpha bias

        for (u32 i = 0; i < m_random_spawn_count; i++)
        {
            if (m_time_counter < m_spawn_data_vector[i].spawn_time)
            {
                continue;
            }

            auto const particle_parent = Entity::create("PARTICLE_PARENT");
            auto const particle = Entity::create("PARTICLE_");

            if (m_simulate_in_world_space)
            {
                particle_parent->transform->set_parent(entity->transform);
            }
            else
            {
                particle_parent->transform->set_position(entity->transform->get_position());
            }

            particle->transform->set_parent(particle_parent->transform);

            auto const particle_comp = particle->add_component(Particle::create(m_spawn_data_vector[i], emitter_bounds, sprite_path));

            // Adjust scale
            float const scale_factor = AK::random_float(min_particle_size, max_particle_size);
            particle->transform->set_local_scale({scale_factor, scale_factor, scale_factor});

            m_spawn_data_vector.erase(m_spawn_data_vector.begin() + i);
            m_random_spawn_count -= 1;
        }
    }

    m_time_counter += delta_time;
}

void ParticleSystem::spawn_calculations()
{
    m_spawn_data_vector.clear();
    m_random_spawn_count = AK::random_int(min_spawn_count, max_spawn_count);

    for (u32 i = 0; i < m_random_spawn_count; i++)
    {
        ParticleSpawnData data = {};

        data.spawn_time = AK::random_float(min_spawn_interval, max_spawn_interval);
        data.spawn_alpha = AK::random_float(min_spawn_alpha, max_spawn_alpha);
        data.start_velocity = glm::linearRand(start_velocity_1, start_velocity_2);
        data.lifetime = AK::random_float(lifetime_1, lifetime_2);
        data.start_color_1 = start_color_1;
        data.end_color_1 = end_color_1;

        m_spawn_data_vector.emplace_back(data);
    }

    m_time_counter = 0.0;
}
