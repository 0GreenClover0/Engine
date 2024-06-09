#include "Particle.h"

#include "AK/AK.h"
#include "ConstantBufferTypes.h"
#include "Entity.h"
#include "Globals.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "ShaderFactory.h"
#include "Sprite.h"

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<Particle> Particle::create()
{
    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {});
    return particle;
}

std::shared_ptr<Particle> Particle::create(float speed, float initial_alpha, float spawn_bounds)
{
    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {}, speed, initial_alpha, spawn_bounds);
    return particle;
}

Particle::Particle(AK::Badge<Particle>)
{
}

Particle::Particle(AK::Badge<Particle>, float speed, float initial_alpha, float spawn_bounds)
    : m_color(m_color.r, m_color.g, m_color.b, initial_alpha), m_speed(speed), m_spawn_bounds(spawn_bounds)
{
}

void Particle::initialize()
{
    Component::initialize();

    set_can_tick(true);

    D3D11_BUFFER_DESC desc = {};
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = static_cast<UINT>(sizeof(ConstantBufferParticle) + (16 - (sizeof(ConstantBufferParticle) % 16)));
    desc.StructureByteStride = 0;

    HRESULT const hr = RendererDX11::get_instance_dx11()->get_device()->CreateBuffer(&desc, nullptr, &m_constant_buffer_particle);
    assert(SUCCEEDED(hr));

    auto const particle_shader = ResourceManager::get_instance().load_shader("./res/shaders/particle.hlsl", "./res/shaders/particle.hlsl");
    auto const particle_material = Material::create(particle_shader, 1);
    particle_material->casts_shadows = false;
    particle_material->needs_forward_rendering = true;
    particle_material->is_billboard = true; // !

    entity->add_component(Sprite::create(particle_material, "./res/textures/smoke.png"));

    entity->transform->set_scale({0.1f, 0.1f, 0.1f});
    entity->transform->set_local_position({AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds),
                                           AK::random_float(-m_spawn_bounds, m_spawn_bounds)});

    update_particle();
}

void Particle::draw_editor()
{
    Component::draw_editor();

    ImGui::ColorEdit4("Color", glm::value_ptr(m_color));
    update_particle();
}

void Particle::update_particle()
{
    ConstantBufferParticle data = {};
    data.color = m_color;

    D3D11_MAPPED_SUBRESOURCE mapped_resource = {};

    HRESULT const hr = RendererDX11::get_instance_dx11()->get_device_context()->Map(m_constant_buffer_particle, 0, D3D11_MAP_WRITE_DISCARD,
                                                                                    0, &mapped_resource);
    assert(SUCCEEDED(hr));

    CopyMemory(mapped_resource.pData, &data, sizeof(ConstantBufferParticle));

    RendererDX11::get_instance_dx11()->get_device_context()->Unmap(m_constant_buffer_particle, 0);
    RendererDX11::get_instance_dx11()->get_device_context()->PSSetConstantBuffers(4, 1, &m_constant_buffer_particle);
}

void Particle::update()
{
    m_color.a -= delta_time;
    glm::vec3 const p = entity->transform->get_position();
    entity->transform->set_position({p.x, p.y + delta_time, p.z});

    if (m_color.a < 0.01f)
        entity->destroy_immediate();

    update_particle();
}
