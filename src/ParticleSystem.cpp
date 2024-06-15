#include "ParticleSystem.h"

#include "AK/AK.h"
#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Particle.h"

#include <glm/gtc/type_ptr.inl>
#include <imgui.h>
#include <imgui_stdlib.h>

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

void ParticleSystem::draw_editor()
{
    Component::draw_editor();

    ImGui::InputText("Particle sprite", &sprite_path);
    ImGui::ColorEdit4("Particle color", value_ptr(color));
    ImGui::DragFloatRange2("Spawn interval", &min_spawn_interval, &max_spawn_interval, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragFloatRange2("Particle speed", &min_particle_speed, &max_particle_speed, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragFloatRange2("Particle size", &min_particle_size, &max_particle_size, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragFloatRange2("Spawn alpha", &min_spawn_alpha, &max_spawn_alpha, 0.1f, 0.0f, 1.0f);
    ImGui::DragFloat("Emitter size", &emitter_bounds, 0.1f, 0.0f, FLT_MAX);
    ImGui::DragIntRange2("Spawn count", &min_spawn_count, &max_spawn_count, 1, 0, INT_MAX);
}

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

            auto const particle_parent = Entity::create("PARTICLE_PARENT" + AK::generate_guid());
            auto const particle = Entity::create("PARTICLE_" + AK::generate_guid());

            particle_parent->transform->set_parent(entity->transform);
            particle->transform->set_parent(particle_parent->transform);

            particle->add_component(Particle::create(m_spawn_data_vector[i].particle_speed,
                                                     {color.r, color.g, color.b, m_spawn_data_vector[i].spawn_alpha}, emitter_bounds,
                                                     sprite_path));

            // Adjust scale
            float const scale_factor = AK::random_float(min_particle_size, max_particle_size);
            glm::vec3 const scale = particle->transform->get_local_scale() * scale_factor;
            particle->transform->set_local_scale(scale);

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
        data.particle_speed = AK::random_float(min_particle_speed, max_particle_speed);

        m_spawn_data_vector.emplace_back(data);
    }

    m_time_counter = 0.0;
}
