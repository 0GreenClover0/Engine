#include "Particle.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "RendererDX11.h"
#include "ResourceManager.h"

#include <glm/gtc/type_ptr.inl>

#if EDITOR
#include "imgui_stdlib.h"
#endif

std::shared_ptr<Particle> Particle::create()
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, 1.0f, "./res/textures/particle.png", particle_material, true);

    particle->prepare();

    return particle;
}

std::shared_ptr<Particle> Particle::create(ParticleSpawnData const& data, float spawn_bounds, std::string const& sprite_path,
                                           bool const rotate_particle)
{
    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;

    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, spawn_bounds, sprite_path, particle_material, rotate_particle);

    particle->prepare();
    particle->set_data(data);

    return particle;
}

std::shared_ptr<Particle> Particle::create(ParticleSpawnData const& data, float spawn_bounds, std::string const& sprite_path,
                                           bool const rotate_particle, std::shared_ptr<Shader> const& shader)
{
    auto const particle_material = Material::create(shader, 1000, false, false, true);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;
    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, spawn_bounds, sprite_path, particle_material, rotate_particle);

    particle->prepare();
    particle->set_data(data);

    return particle;
}

Particle::Particle(AK::Badge<Particle>, float const spawn_bounds, std::string const& sprite_path, std::shared_ptr<Material> const& mat,
                   bool const rotate_particle)
    : Drawable(mat), rotate(rotate_particle), path(sprite_path), m_particle_material(mat), m_spawn_bounds(spawn_bounds)
{
}

void Particle::awake()
{
    set_can_tick(true);

    entity->transform->set_local_position({AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds)});
    if (rotate)
    {
        entity->transform->set_euler_angles({0, 0, AK::random_float(0.0f, 360.0f)});
    }

    m_rotation_direction = AK::random_bool() ? 1.0f : -1.0f;

    update_particle();

    if (!entity->transform->parent.expired())
    {
        entity->transform->parent.lock()->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
    }
    else
    {
        entity->transform->set_euler_angles(Camera::get_main_camera()->entity->transform->get_euler_angles());
    }

    m_random_seed = AK::random_float(-1.0f, 1.0f);

    m_original_position = entity->transform->get_position();
}

void Particle::draw() const
{
    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    if (!entity->transform->parent.expired())
    {
        entity->transform->parent.lock()->set_rotation(Camera::get_main_camera()->entity->transform->get_euler_angles());
    }
    else
    {
        entity->transform->set_rotation(Camera::get_main_camera()->entity->transform->get_euler_angles());
    }

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
    ImGui::InputText("Sprite Path", &path);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        reprepare();
    }

    update_particle();
}
#endif

void Particle::reprepare()
{
    Drawable::reprepare();

    prepare();
}

void Particle::update_particle() const
{
    m_particle_material->color = m_color;
}

bool Particle::update_lifetime()
{
    m_current_lifetime += static_cast<float>(delta_time);

    if (m_current_lifetime >= m_lifetime)
    {
        if (!entity->transform->parent.expired())
        {
            entity->transform->parent.lock()->entity.lock()->destroy_immediate();
        }
        else if (entity != nullptr)
        {
            entity->destroy_immediate();
        }

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
    glm::vec3 new_position;
    glm::vec3 change = {};

    switch (particle_type)
    {
    case ParticleType::Prompt:
    {
        change.y = sin(m_current_lifetime * 5.0f) * 0.1f;
        new_position = m_original_position + change;
        break;
    }
    case ParticleType::Snow:
    {
        glm::vec3 const p = entity->transform->get_position();
        change.x = sin(static_cast<float>(glfwGetTime()) + m_random_seed * 1.5f) * 0.035f - static_cast<float>(delta_time) * 1.7f;
        change.z = change.x;

        // Ensure the y-component decreases over time for downward motion
        change.y = -static_cast<float>(delta_time) * 6.5f;
        new_position = p + change;
        break;
    }
    default:
    {
        glm::vec3 const p = entity->transform->get_position();
        change = m_velocity * static_cast<float>(delta_time);
        new_position = p + change;
        break;
    }
    }

    entity->transform->set_position(new_position);
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
    m_mesh = create_sprite();
}

void Particle::set_data(ParticleSpawnData const& data)
{
    m_lifetime = data.lifetime;
    m_velocity = data.start_velocity;
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

    if (!path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(0, path, vertices, indices, textures, DrawType::Triangles, material);
}
