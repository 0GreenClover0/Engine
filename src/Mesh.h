#pragma once

#include <vector>
#include <cstdint>

#include "Bounds.h"
#include "Drawable.h"
#include "DrawType.h"
#include "Texture.h"
#include "Vertex.h"

class Mesh
{
public:
    virtual ~Mesh() = default;

    void virtual draw() const = 0;
    void virtual draw(uint32_t const size, void const* offset) const = 0;
    void virtual draw_instanced(int32_t const size) const = 0;

    void virtual bind_textures() const = 0;
    void virtual unbind_textures() const = 0;

    void calculate_bounding_box();
    void adjust_bounding_box(glm::mat4 const& model_matrix);
    [[nodiscard]] BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    BoundingBox bounds = {};

    std::shared_ptr<Material> material;

protected:
    Mesh(std::vector<Vertex> const& vertices, std::vector<std::uint32_t> const& indices, std::vector<Texture> const& textures,
         DrawType const draw_type, std::shared_ptr<Material> const& material, DrawFunctionType const draw_function);

    [[nodiscard]] BoundingBox calculate_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    DrawType draw_type;
    DrawFunctionType draw_function;
};
