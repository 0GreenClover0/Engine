#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <glad/glad.h>

#include "Bounds.h"
#include "Drawable.h"
#include "Texture.h"
#include "Vertex.h"

class Mesh
{
public:
    Mesh() = delete;
    ~Mesh();
    Mesh(Mesh&& mesh) noexcept;

    static Mesh create(std::vector<Vertex> const& vertices, std::vector<std::uint32_t> const& indices,
                       std::vector<Texture> const& textures, GLenum draw_type, std::shared_ptr<Material> const& material);

    void draw() const;

    void bind_textures() const;
    void draw_instanced(int32_t const size) const;
    void unbind_textures() const;

    void calculate_bounding_box();
    void adjust_bounding_box(glm::mat4 const& model_matrix);
    BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    BoundingBox bounds = {};

    GLenum draw_type;
    std::shared_ptr<Material> material;

protected:
    Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices, std::vector<Texture> textures, GLenum draw_type, std::shared_ptr<Material> const& material);
    void setup_mesh();

    [[nodiscard]] BoundingBox calculate_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

private:
    std::uint32_t VAO = {}, VBO = {}, EBO = {};
};
