#include "MeshDX11.h"

#include <array>
#include <iostream>

#include "RendererDX11.h"
#include <TextureLoader.h>
#include <TextureLoaderDX11.h>

MeshDX11::MeshDX11(AK::Badge<MeshFactory>, std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
                   std::vector<Texture> const& textures, DrawType const draw_type, std::shared_ptr<Material> const& material,
                   DrawFunctionType const draw_function) : Mesh(vertices, indices, textures, draw_type, material, draw_function)
{
    switch (draw_type)
    {
    case DrawType::Triangles:
        m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    case DrawType::TriangleStrip:
        m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        break;
    case DrawType::Patches:
        std::cout << "DX11 'Patches' draw type is not supported.\n";
        break;
    case DrawType::TriangleFan:
        // https://gamedev.stackexchange.com/questions/35547/why-are-triangle-fans-not-supported-in-direct3d-10-or-later
        std::cout << "DX11 'TriangleFan' draw type is not supported.\n";
        break;
    case DrawType::LineLoop:
        std::cout << "DX11 'LineLoop' draw type is not supported.\n";
        break;
    case DrawType::LineStrip:
        m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        break;
    case DrawType::Lines:
        m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    case DrawType::Points:
        m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        break;
    default:
        std::unreachable();
    }

    ID3D11Device* device = RendererDX11::get_instance_dx11()->get_device();

    m_vertex_buffer = std::make_shared<VertexBufferDX11>(device, vertices.data(), vertices.size());
    m_index_buffer = std::make_shared<IndexBufferDX11>(device, indices.data(), indices.size());
}

MeshDX11::MeshDX11(MeshDX11&& mesh) noexcept : Mesh(mesh.m_vertices, mesh.m_indices, mesh.m_textures, mesh.m_draw_type, mesh.material, mesh.m_draw_function)
{
    m_vertex_buffer = mesh.m_vertex_buffer;
    mesh.m_vertex_buffer = nullptr;

    m_index_buffer = mesh.m_index_buffer;
    mesh.m_index_buffer = nullptr;

    mesh.m_vertices.clear();
    mesh.m_indices.clear();
    mesh.m_textures.clear();
}

MeshDX11::~MeshDX11()
{
    m_vertices.clear();
    m_indices.clear();
    m_textures.clear();
}

void MeshDX11::draw() const
{
    bind_textures();

    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();

    u32 constexpr offset = 0;
    device_context->IASetPrimitiveTopology(m_primitive_topology);
    device_context->IASetVertexBuffers(0, 1, m_vertex_buffer->get_address_of(), m_vertex_buffer->stride_ptr(), &offset);
    device_context->IASetIndexBuffer(m_index_buffer->get(), DXGI_FORMAT_R32_UINT, 0);
    device_context->DrawIndexed(m_index_buffer->buffer_size(), 0, 0);

    unbind_textures();
}

void MeshDX11::draw(u32 const size, void const* offset) const
{
}

void MeshDX11::draw_instanced(i32 const size) const
{
}

void MeshDX11::bind_textures() const
{
    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();

    for (u32 i = 0; i < m_textures.size(); ++i)
    {
        device_context->PSSetShaderResources(0, 1, &m_textures[i].shader_resource_view);
        device_context->PSSetSamplers(0, 1, &m_textures[i].image_sampler_state);
    }
}

void MeshDX11::unbind_textures() const
{
    auto const device_context = RendererDX11::get_instance_dx11()->get_device_context();

    ID3D11ShaderResourceView* null_shader_resource_view = nullptr;
    for (u32 i = 0; i < m_textures.size(); ++i)
    {
        device_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
        device_context->PSSetSamplers(0, 1, &m_textures[i].image_sampler_state);
    }
}
