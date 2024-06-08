#include "Particle.h"

#include "ConstantBufferTypes.h"
#include "Entity.h"
#include "Globals.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "Sprite.h"

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<Particle> Particle::create()
{
    auto particle = std::make_shared<Particle>(AK::Badge<Particle> {});
    return particle;
}

Particle::Particle(AK::Badge<Particle>)
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

    entity->add_component(Sprite::create(particle_material, "./res/textures/smoke.png"));
}

void Particle::draw_editor()
{
    Component::draw_editor();

    ImGui::ColorEdit4("Color", glm::value_ptr(color));
    update_particle();
}

void Particle::update_particle()
{
    ConstantBufferParticle data = {};
    data.color = color;

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
    update_particle();
}
