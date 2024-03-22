#pragma once

#include <d3d11.h>

#include "IndexBuffer.h"
#include "Mesh.h"
#include "VertexBuffer.h"

class MeshFactory;

class MeshDX11 final : public Mesh
{
public:
    MeshDX11(AK::Badge<MeshFactory>, std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
             std::vector<Texture> const& textures, DrawType const draw_type, std::shared_ptr<Material> const& material,
             DrawFunctionType const draw_function);

    MeshDX11(MeshDX11&& mesh) noexcept;
    // ~MeshDX11() override;


    void virtual draw() const override;
    void virtual draw(u32 const size, void const* offset) const override;
    void virtual draw_instanced(i32 const size) const override;

    void virtual bind_textures() const override;
    void virtual unbind_textures() const override;

private:
    VertexBuffer<Vertex> m_vertex_buffer;
    IndexBuffer m_index_buffer;

    D3D_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
