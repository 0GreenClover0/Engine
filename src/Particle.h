#pragma once

#include "Drawable.h"
#include "GBuffer.h"

class Particle final : public Drawable
{
public:
    static std::shared_ptr<Particle> create();
    static std::shared_ptr<Particle> create(float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path);
    explicit Particle(AK::Badge<Particle>, float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path,
                      std::shared_ptr<Material> const& mat);

    virtual void initialize() override;

    virtual void update() override;

    virtual bool is_particle() const override;

    virtual void draw() const override;
    virtual void draw_editor() override;

protected:
    void move() const;
    void update_particle() const;
    void decrement_alpha();

    std::shared_ptr<Material> m_particle_material = {};

    ID3D11Buffer* m_constant_buffer_particle = nullptr;

    glm::vec4 m_color = {1.0f, 1.0f, 1.0f, 1.0f};
    float m_speed = 1.0f;
    float m_spawn_bounds = 1.0f;
    float m_rotation_direction = 1.0f;
    std::string m_path = "./res/textures/particle.png";
};
