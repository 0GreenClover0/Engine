#pragma once

#include "Mesh.h"

class MeshFactory;

class MeshGL final : public Mesh
{
public:
    MeshGL(AK::Badge<MeshFactory>, std::vector<Vertex> const& vertices, std::vector<std::uint32_t> const& indices,
           std::vector<Texture> const& textures, DrawType const draw_type, std::shared_ptr<Material> const& material,
           DrawFunctionType const draw_function);

    MeshGL(MeshGL&& mesh) noexcept;
    ~MeshGL() override;

    void draw() const override;
    void draw(uint32_t const size, void const* offset) const override;
    void draw_instanced(int32_t const size) const override;

    void bind_textures() const override;
    void unbind_textures() const override;

private:
    uint32_t m_draw_typeGL = 0;

    uint32_t m_VAO = 0, m_VBO = 0, m_EBO = 0;
};
