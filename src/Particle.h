#pragma once

#include "Drawable.h"
#include "GBuffer.h"

class Particle final : public Component
{
public:
    static std::shared_ptr<Particle> create();
    static std::shared_ptr<Particle> create(float speed, float initial_alpha, float spawn_bounds);
    explicit Particle(AK::Badge<Particle>);
    explicit Particle(AK::Badge<Particle>, float speed, float initial_alpha, float spawn_bounds);

    virtual void initialize() override;

    virtual void update() override;

    virtual void draw_editor() override;

private:
    void update_particle();

    ID3D11Buffer* m_constant_buffer_particle = nullptr;

    glm::vec4 m_color = {0.0f, 0.0f, 1.0f, 1.0f};
    float m_speed = 1.0f;
    float m_spawn_bounds = 1.0f;
};
