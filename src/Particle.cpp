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

    glm::vec4 color = {1.0f, 1.0f, 1.0f, 1.0f};

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, 1.0f, color, 1.0f, "./res/textures/particle.png", particle_material);

    particle->prepare();

    return particle;
}

std::shared_ptr<Particle> Particle::create(float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path)
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, speed, color, spawn_bounds, path, particle_material);

    particle->prepare();

    return particle;
}

Particle::Particle(AK::Badge<Particle>, float speed, glm::vec4 const& color, float spawn_bounds, std::string const& path,
                   std::shared_ptr<Material> const& mat)
    : Drawable(mat), m_particle_material(mat), m_color(color), m_speed(speed), m_spawn_bounds(spawn_bounds), m_path(path)
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

    if (!entity->transform->parent.expired())
        entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
    else
        entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
}

void Particle::draw() const
{
    if (!entity->transform->parent.expired())
        entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
    else
        entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());

    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    if (m_mesh != nullptr)
    {
        m_mesh->draw();
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
    entity->transform->set_position({p.x, p.y + static_cast<float>(delta_time) * m_speed, p.z});

    // Rotate
    glm::vec3 const rot = entity->transform->get_euler_angles();
    entity->transform->set_euler_angles({rot.x, rot.y, rot.z + m_speed * 0.4f * m_rotation_direction});
}

void Particle::decrement_alpha()
{
    m_color.a -= delta_time;

    if (m_color.a < 0.01f)
    {
        if (!entity->transform->parent.expired())
            entity->transform->parent.lock()->entity.lock()->destroy_immediate();
        else
            destroy_immediate();
    }
}

bool Particle::is_particle() const
{
    return true;
}

void Particle::prepare()
{
    m_mesh = create_sprite();
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
