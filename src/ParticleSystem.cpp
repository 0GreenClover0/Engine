#include "ParticleSystem.h"

#include "AK/AK.h"
#include "Entity.h"
#include "Globals.h"
#include "Particle.h"

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

void ParticleSystem::update()
{
    if (m_spawn_data_vector.empty())
    {
        spawn_calculations();
    }
    else
    {
        for (u32 i = 0; i < m_random_spawn_count; i++)
        {
            if (m_time_counter < m_spawn_data_vector[i].spawn_time)
            {
                continue;
            }

            auto const particle = Entity::create("PARTICLE_" + AK::generate_guid());
            particle->add_component(
                Particle::create(m_spawn_data_vector[i].particle_speed, m_spawn_data_vector[i].spawn_alpha, emitter_bounds));

            particle->transform->set_parent(entity->transform);
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
