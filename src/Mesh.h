#pragma once

#include <vector>

#include "AK/Types.h"
#include "Bounds.h"
#include "DrawType.h"
#include "Drawable.h"
#include "Texture.h"
#include "Vertex.h"

class Mesh
{
public:
    virtual ~Mesh() = default;

    void virtual draw() const = 0;
    void virtual draw(u32 const size, void const* offset) const = 0;
    void virtual draw_instanced(i32 const size) const = 0;

    void virtual bind_textures() const = 0;
    void virtual unbind_textures() const = 0;

    void calculate_bounding_box();
    void adjust_bounding_box(glm::mat4 const& model_matrix);
    [[nodiscard]] BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    BoundingBox bounds = {};

    std::shared_ptr<Material> material;

protected:
    Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<std::shared_ptr<Texture>> const& textures,
         DrawType const draw_type, std::shared_ptr<Material> const& material, DrawFunctionType const draw_function);

    [[nodiscard]] BoundingBox calculate_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    std::vector<Vertex> m_vertices;
    std::vector<u32> m_indices;
    std::vector<std::shared_ptr<Texture>> m_textures;

    DrawType m_draw_type;
    DrawFunctionType m_draw_function;
};
