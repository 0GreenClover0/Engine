#pragma once

#include "Drawable.h"
#include "GBuffer.h"
#include "ParticleSystem.h"

class Mesh;

class Particle final : public Drawable
{
public:
    static std::shared_ptr<Particle> create();
    static std::shared_ptr<Particle> create(ParticleSpawnData const& data, float spawn_bounds, std::string const& path);
    explicit Particle(AK::Badge<Particle>, float spawn_bounds, std::string const& path, std::shared_ptr<Material> const& mat);

    virtual void awake() override;

    virtual void update() override;

    virtual bool is_particle() const override;

    virtual void draw() const override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    void prepare();

    void set_data(ParticleSpawnData const& data);

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;

    void move() const;
    void update_particle() const;
    bool update_lifetime();
    void interpolate_color();

    std::shared_ptr<Material> m_particle_material = {};

    ID3D11Buffer* m_constant_buffer_particle = nullptr;

    float m_current_lifetime = 0.0f;
    glm::vec4 m_color = {1.0f, 1.0f, 1.0f, 1.0f};

    glm::vec4 m_start_color_1 = {};
    glm::vec4 m_end_color_1 = {};
    glm::vec3 m_velocity = {};
    float m_spawn_bounds = 1.0f;
    float m_rotation_direction = 1.0f;
    float m_lifetime = 5.0f;
    std::string m_path = "./res/textures/particle.png";

    std::shared_ptr<Mesh> mesh = {};
};
