#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

struct ParticleSpawnData
{
    float spawn_time = 0.0f;
    float spawn_alpha = 0.0f;
    float particle_speed = 0.0f;
};

class ParticleSystem final : public Component
{
public:
    static std::shared_ptr<ParticleSystem> create();
    explicit ParticleSystem(AK::Badge<ParticleSystem>);

    virtual void awake() override;
    virtual void update() override;

    float min_spawn_interval = 0.5f;
    float max_spawn_interval = 1.0f;

    float min_particle_speed = 1.0f;
    float max_particle_speed = 2.0f;

    float min_spawn_alpha = 0.5f;
    float max_spawn_alpha = 0.8f;

    float emitter_bounds = 0.1f;

    i32 min_spawn_count = 1;
    i32 max_spawn_count = 3;

private:
    void spawn_calculations();

    std::vector<ParticleSpawnData> m_spawn_data_vector = {};

    u32 m_random_spawn_count = 0;
    double m_time_counter = 0.0;
    double m_spawn_interval = 0.0;
};
