#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
#include <glad/glad.h>

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
                       std::vector<Texture> const& textures, GLenum draw_type, std::shared_ptr<MaterialInstance> const& material_instance);

    void draw() const;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    GLenum draw_type;
    std::shared_ptr<MaterialInstance> material_instance;

protected:
    Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices, std::vector<Texture> textures, GLenum draw_type, std::shared_ptr<MaterialInstance> const& material_instance);
    void setup_mesh();

private:
    std::uint32_t VAO = {}, VBO = {}, EBO = {};
};
