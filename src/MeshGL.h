#pragma once

#include "Mesh.h"

class MeshFactory;

class MeshGL final : public Mesh
{
public:
    MeshGL(AK::Badge<MeshFactory>, std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
           std::vector<std::shared_ptr<Texture>> const& textures, DrawType const draw_type, std::shared_ptr<Material> const& material,
           DrawFunctionType const draw_function);

    MeshGL(MeshGL&& mesh) noexcept;
    ~MeshGL() override;

    virtual void draw() const override;
    virtual void draw(u32 const size, void const* offset) const override;
    virtual void draw_instanced(i32 const size) const override;

    virtual void bind_textures() const override;
    virtual void unbind_textures() const override;

private:
    u32 m_draw_typeGL = 0;

    u32 m_VAO = 0, m_VBO = 0, m_EBO = 0;
};
