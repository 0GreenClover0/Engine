#pragma once

#include "Drawable.h"
#include "GBuffer.h"
#include "ParticleSystem.h"

class Mesh;

NON_SERIALIZED
class Particle final : public Drawable
{
public:
    static std::shared_ptr<Particle> create();
    static std::shared_ptr<Particle> create(ParticleSpawnData const& data, float spawn_bounds, std::string const& sprite_path,
                                            bool const rotate_particle);
    static std::shared_ptr<Particle> create(ParticleSpawnData const& data, float spawn_bounds, std::string const& sprite_path,
                                            bool const rotate_particle, std::shared_ptr<Shader> const& shader);
    explicit Particle(AK::Badge<Particle>, float const spawn_bounds, std::string const& sprite_path, std::shared_ptr<Material> const& mat,
                      bool const rotate_particle);

    virtual void awake() override;
    virtual void update() override;
    virtual bool is_particle() const override;
    virtual void draw() const override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void reprepare() override;
    void prepare();

    void set_data(ParticleSpawnData const& data);

    NON_SERIALIZED
    bool rotate = true;

    NON_SERIALIZED
    ParticleType particle_type = ParticleType::Default;

    std::string path = "./res/textures/particle.png";

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;

    void move() const;
    void update_particle() const;
    bool update_lifetime();
    void interpolate_color();

    std::shared_ptr<Material> m_particle_material = {};

    ID3D11Buffer* m_constant_buffer_particle = nullptr;

    glm::vec3 m_original_position = {};
    glm::vec3 m_fish_offset = {};

    float m_current_lifetime = 0.0f;
    glm::vec4 m_color = {1.0f, 1.0f, 1.0f, 1.0f};

    glm::vec4 m_start_color_1 = {};
    glm::vec4 m_end_color_1 = {};
    glm::vec3 m_velocity = {};
    float m_spawn_bounds = 1.0f;
    float m_rotation_direction = 1.0f;
    float m_lifetime = 5.0f;
    float m_random_seed = 0.0f;

    glm::vec3 m_customer_group_position = {};

    std::shared_ptr<Mesh> m_mesh = {};
};
