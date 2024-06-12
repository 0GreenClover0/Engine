#include "Particle.h"

#include "AK/AK.h"
#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "Sprite.h"

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<Particle> Particle::create()
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, 1.0f, color, 1.0f, "./res/textures/particle.png", particle_material);
    return particle;
}

std::shared_ptr<Particle> Particle::create(float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path)
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, speed, color, spawn_bounds, path, particle_material);
    return particle;
}

Particle::Particle(AK::Badge<Particle>, float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path,
                   std::shared_ptr<Material> const& mat)
    : Drawable(mat), m_particle_material(mat), m_color(color), m_speed(speed), m_spawn_bounds(spawn_bounds), m_path(path)
{
}

void Particle::initialize()
{
    Drawable::initialize();

    set_can_tick(true);

    entity->add_component(Sprite::create(m_particle_material, m_path));

    entity->transform->set_scale({0.1f, 0.1f, 0.1f});
    entity->transform->set_local_position({AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds)});
    entity->transform->set_euler_angles({0, 0, AK::random_float(0.0f, 360.0f)});

    m_rotation_direction = AK::random_bool() ? 1.0f : -1.0f;

    update_particle();
}

void Particle::draw() const
{
    entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
}

void Particle::draw_editor()
{
    Drawable::draw_editor();

    ImGui::ColorEdit4("Color", glm::value_ptr(m_color));
    update_particle();
}

void Particle::update_particle() const
{
    m_particle_material->color = m_color;
}

void Particle::update()
{
    move();
    decrement_alpha();
    update_particle();
}

void Particle::move() const
{
    if (entity == nullptr)
    {
        return;
    }

    // Move up
    glm::vec3 const p = entity->transform->get_position();
    entity->transform->set_position({p.x, p.y + delta_time * m_speed, p.z});

    // Rotate
    glm::vec3 const rot = entity->transform->get_euler_angles();
    entity->transform->set_euler_angles({rot.x, rot.y, rot.z + m_speed * 0.4f * m_rotation_direction});
}

void Particle::decrement_alpha()
{
    m_color.a -= delta_time;

    if (m_color.a < 0.01f)
    {
        entity->transform->parent.lock()->entity.lock()->destroy_immediate();
    }
}

bool Particle::is_particle() const
{
    return true;
}
