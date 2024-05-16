#include "FullscreenQuad.h"

#include <array>

#include "Vertex.h"
#include "RendererDX11.h"
#include "ResourceManager.h"

FullscreenQuad::FullscreenQuad(AK::Badge<FullscreenQuad>)
{
    std::array quad_vertices =
    {
        // positions        // normals     // texture Coords
        Vertex(glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec3(1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f))
    };

    std::array<u32, 6> quad_indices =
    {
        0, 1, 2,
        2, 1, 3,
    };

    m_vertex_buffer = std::make_shared<VertexBufferDX11>(RendererDX11::get_instance_dx11()->get_device(), quad_vertices.data(), quad_vertices.size());
    m_index_buffer = std::make_shared<IndexBufferDX11>(RendererDX11::get_instance_dx11()->get_device(), quad_indices.data(), quad_indices.size());
}

std::shared_ptr<FullscreenQuad> FullscreenQuad::create()
{
    auto fullscreen_quad =  std::make_shared<FullscreenQuad>(AK::Badge<FullscreenQuad>{});

    set_instance(fullscreen_quad);

    return fullscreen_quad;
}

void FullscreenQuad::draw() const
{
    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();
    u32 constexpr offset = 0;
    device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->IASetVertexBuffers(0, 1, m_vertex_buffer->get_address_of(), m_vertex_buffer->stride_ptr(), &offset);
    device_context->IASetIndexBuffer(m_index_buffer->get(), DXGI_FORMAT_R32_UINT, 0);
    device_context->DrawIndexed(6, 0, 0);
}

std::shared_ptr<FullscreenQuad> FullscreenQuad::get_instance()
{
    if (m_instance == nullptr)
    {
        create();
    }

    return m_instance;
}

void FullscreenQuad::set_instance(std::shared_ptr<FullscreenQuad> const& fullscreen_quad)
{
    m_instance = fullscreen_quad;
}
