#include "Particle.h"

#include "AK/AK.h"
#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "RendererDX11.h"
#include "ResourceManager.h"

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<Particle> Particle::create()
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, 1.0f, "./res/textures/particle.png", particle_material);

    particle->prepare();

    return particle;
}

std::shared_ptr<Particle> Particle::create(ParticleSpawnData const& data, float spawn_bounds, std::string const& path)
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, spawn_bounds, path, particle_material);

    particle->prepare();
    particle->set_data(data);

    return particle;
}

Particle::Particle(AK::Badge<Particle>, float spawn_bounds, std::string const& path, std::shared_ptr<Material> const& mat)
    : Drawable(mat), m_particle_material(mat), m_spawn_bounds(spawn_bounds), m_path(path)
{
}

void Particle::awake()
{
    set_can_tick(true);

    entity->transform->set_scale({0.1f, 0.1f, 0.1f});
    entity->transform->set_local_position({AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds)});
    entity->transform->set_euler_angles({0, 0, AK::random_float(0.0f, 360.0f)});

    m_rotation_direction = AK::random_bool() ? 1.0f : -1.0f;

    update_particle();

    entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
}

void Particle::draw() const
{
    entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());

    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    if (mesh != nullptr)
    {
        mesh->draw();
    }

    Renderer::get_instance()->restore_default_rasterizer_draw_type();
}

#if EDITOR
void Particle::draw_editor()
{
    Drawable::draw_editor();

    ImGui::ColorEdit4("Color", glm::value_ptr(m_color));
    update_particle();
}
#endif

void Particle::update_particle() const
{
    m_particle_material->color = m_color;
}

bool Particle::update_lifetime()
{
    m_current_lifetime += static_cast<float>(delta_time);

    if (m_current_lifetime >= m_lifetime)
    {
        entity->transform->parent.lock()->entity.lock()->destroy_immediate();
        return true;
    }

    return false;
}

void Particle::update()
{
    if (update_lifetime())
    {
        return;
    }

    move();

    interpolate_color();

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
    entity->transform->set_position({p.x, p.y + static_cast<float>(delta_time) * m_speed, p.z});

    // Rotate
    glm::vec3 const rot = entity->transform->get_euler_angles();
    entity->transform->set_euler_angles({rot.x, rot.y, rot.z + m_speed * 0.4f * m_rotation_direction});
}

void Particle::interpolate_color()
{
    m_color = AK::interpolate_color(m_start_color_1, m_end_color_1, m_current_lifetime / m_lifetime);
}

bool Particle::is_particle() const
{
    return true;
}

void Particle::prepare()
{
    mesh = create_sprite();
}

void Particle::set_data(ParticleSpawnData const& data)
{
    m_lifetime = data.lifetime;
    m_speed = data.particle_speed;
    m_color = data.start_color_1;
    m_start_color_1 = data.start_color_1;
    m_end_color_1 = data.end_color_1;
}

std::shared_ptr<Mesh> Particle::create_sprite() const
{
    std::vector<Vertex> const vertices = {
        {glm::vec3(-1.0f, -1.0f, 0.0f), {}, {0.0f, 0.0f}}, // bottom left
        {glm::vec3(1.0f, -1.0f, 0.0f), {}, {1.0f, 0.0f}}, // bottom right
        {glm::vec3(1.0f, 1.0f, 0.0f), {}, {1.0f, 1.0f}}, // top right
        {glm::vec3(-1.0f, 1.0f, 0.0f), {}, {0.0f, 1.0f}}, // top left
    };

    std::vector<u32> const indices = {0, 1, 2, 0, 2, 3};

    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};
    TextureSettings texture_settings = {};
    texture_settings.wrap_mode_x = TextureWrapMode::ClampToEdge;
    texture_settings.wrap_mode_y = TextureWrapMode::ClampToEdge;

    if (!m_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(m_path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(0, m_path, vertices, indices, textures, DrawType::Triangles, material);
}
