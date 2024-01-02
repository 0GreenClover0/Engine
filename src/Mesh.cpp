#include "Mesh.h"

#include <iostream>
#include <string>
#include <utility>
#include <glad/glad.h>
#include <glm/gtc/epsilon.hpp>

#include "Globals.h"
#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<std::uint32_t> indices, std::vector<Texture> textures,
           GLenum draw_type, std::shared_ptr<Material> const& material, DrawFunctionType const draw_function)
    : vertices(std::move(vertices)), indices(std::move(indices)), textures(std::move(textures)), draw_type(draw_type), material(material),
      draw_function(draw_function)
{
}

void Mesh::setup_mesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

    // FIXME: Not all shaders have all these attributes

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Vertex texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinates));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::calculate_bounding_box()
{
    if (vertices.empty())
        return;

    float lowest_x = vertices[0].position.x;
    float lowest_y = vertices[0].position.y;
    float lowest_z = vertices[0].position.z;
    float highest_x = vertices[0].position.x;
    float highest_y = vertices[0].position.y;
    float highest_z = vertices[0].position.z;

    for (auto const& vertex : vertices)
    {
        if (vertex.position.x < lowest_x)
            lowest_x = vertex.position.x;
        if (vertex.position.y < lowest_y)
            lowest_y = vertex.position.y;
        if (vertex.position.z < lowest_z)
            lowest_z = vertex.position.z;

        if (vertex.position.x > highest_x)
            highest_x = vertex.position.x;
        if (vertex.position.y > highest_y)
            highest_y = vertex.position.y;
        if (vertex.position.z > highest_z)
            highest_z = vertex.position.z;
    }

    this->bounds =
    {
        glm::vec3(lowest_x, lowest_y, lowest_z),
        glm::vec3(highest_x, highest_y, highest_z)
    };
}

void Mesh::adjust_bounding_box(glm::mat4 const& model_matrix)
{
    this->bounds = calculate_adjusted_bounding_box(model_matrix);
}

BoundingBox Mesh::get_adjusted_bounding_box(glm::mat4 const& model_matrix) const
{
    return calculate_adjusted_bounding_box(model_matrix);
}

BoundingBox Mesh::calculate_adjusted_bounding_box(glm::mat4 const& model_matrix) const
{
    // OPTIMIZATION: For uniformly scaled objects we can perform only 2 multiplications instead of a full matrix one
    // to determine the new bounding box. This is taken from:
    // https://stackoverflow.com/questions/6053522/how-to-recalculate-axis-aligned-bounding-box-after-translate-rotate
    // and:
    // https://github.com/erich666/GraphicsGems/blob/master/gems/TransBox.c
    auto const translation = glm::vec3(model_matrix[3]);
    glm::mat3 const rotation = model_matrix;
    auto const scale = glm::vec3(glm::length(rotation[0]), glm::length(rotation[1]), glm::length(rotation[2]));
    if (glm::epsilonEqual(scale.x, scale.y, 0.0001f) && glm::epsilonEqual(scale.x, scale.z, 0.0001f))
    {
        float min[3];
        float max[3];
        min[0] = max[0] = translation.x;
        min[1] = max[1] = translation.y;
        min[2] = max[2] = translation.z;

        for (uint32_t i = 0; i < 3; ++i)
        {
            for (uint32_t k = 0; k < 3; ++k)
            {
                float a = rotation[i][k] * bounds.min[k];
                float b = rotation[i][k] * bounds.max[k];
                min[i] += a < b ? a : b;
                max[i] += a < b ? b : a;
            }
        }

        return { glm::vec3(min[0], min[1], min[2]), glm::vec3(max[0], max[1], max[2]) };
    }

    // Create AABB vertices from bounds
    std::vector<glm::vec3> aabb_vertices =
    {
        bounds.min,
        glm::vec3(bounds.min.x, bounds.min.y, bounds.max.z),
        glm::vec3(bounds.min.x, bounds.max.y, bounds.min.z),
        glm::vec3(bounds.min.x, bounds.max.y, bounds.max.z),
        glm::vec3(bounds.max.x, bounds.min.y, bounds.min.z),
        glm::vec3(bounds.max.x, bounds.min.y, bounds.max.z),
        glm::vec3(bounds.max.x, bounds.max.y, bounds.min.z),
        bounds.max
    };

    // Transform AABB vertices by model matrix
    for (auto& vertex : aabb_vertices)
    {
        vertex = glm::vec3(model_matrix * glm::vec4(vertex, 1.0f));
    }

    // Find new AABB bounds
    float lowest_x = aabb_vertices[0].x;
    float lowest_y = aabb_vertices[0].y;
    float lowest_z = aabb_vertices[0].z;
    float highest_x = aabb_vertices[0].x;
    float highest_y = aabb_vertices[0].y;
    float highest_z = aabb_vertices[0].z;

    for (auto const& vertex : aabb_vertices)
    {
        if (vertex.x < lowest_x)
            lowest_x = vertex.x;
        if (vertex.y < lowest_y)
            lowest_y = vertex.y;
        if (vertex.z < lowest_z)
            lowest_z = vertex.z;

        if (vertex.x > highest_x)
            highest_x = vertex.x;
        if (vertex.y > highest_y)
            highest_y = vertex.y;
        if (vertex.z > highest_z)
            highest_z = vertex.z;
    }

    return { glm::vec3(lowest_x, lowest_y, lowest_z), glm::vec3(highest_x, highest_y, highest_z) };
}

Mesh::~Mesh()
{
    for (auto const& texture : textures)
    {
        glDeleteTextures(1, &texture.id);
    }

    vertices.clear();
    indices.clear();
    textures.clear();

    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

Mesh::Mesh(Mesh&& mesh) noexcept : Mesh(mesh.vertices, mesh.indices, mesh.textures, mesh.draw_type, mesh.material, mesh.draw_function)
{
    VAO = mesh.VAO;
    VBO = mesh.VBO;
    EBO = mesh.EBO;

    mesh.VAO = 0;
    mesh.VBO = 0;
    mesh.EBO = 0;

    mesh.vertices.clear();
    mesh.indices.clear();
    mesh.textures.clear();
}

Mesh Mesh::create(std::vector<Vertex> const& vertices, std::vector<std::uint32_t> const& indices, std::vector<Texture> const& textures,
                  GLenum const draw_type, std::shared_ptr<Material> const& material, DrawFunctionType const draw_function)
{
    auto mesh = Mesh(vertices, indices, textures, draw_type, material, draw_function);
    mesh.setup_mesh();
    return mesh;
}

void Mesh::draw() const
{
    bind_textures();

    // Draw mesh
    glBindVertexArray(VAO);

    if (draw_function == DrawFunctionType::NotIndexed)
        glDrawArrays(draw_type, 0, static_cast<int>(vertices.size()));
    else
        glDrawElements(draw_type, static_cast<int>(indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    unbind_textures();
}

void Mesh::draw(uint32_t const size, void const* offset) const
{
    bind_textures();

    glBindVertexArray(VAO);

    if (draw_function == DrawFunctionType::Indexed)
    {
        glDrawElements(draw_type, size, GL_UNSIGNED_INT, offset);
    }
    else
    {
        std::cout << "Non indexed drawing with offset is not currently supported." << "\n";
    }

    glBindVertexArray(0);

    unbind_textures();
}

void Mesh::bind_textures() const
{
    std::uint32_t diffuse_number = 1;
    std::uint32_t specular_number = 1;
    std::uint32_t height_number = 1;

    for (std::uint32_t i = 0; i < textures.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = "material." + textures[i].type;

        if (textures[i].type == "texture_diffuse")
        {
            number = std::to_string(diffuse_number++);
        }
        else if (textures[i].type == "texture_specular")
        {
            number = std::to_string(specular_number++);
        }
        else if (textures[i].type == "texture_height")
        {
            number = std::to_string(height_number++);
        }

        material->shader->set_int(name + number, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    if (textures.empty())
    {
        glActiveTexture(GL_TEXTURE0);

        material->shader->set_int("material.texture_diffuse1", 0);

        glBindTexture(GL_TEXTURE_2D, InternalMeshData::white_texture.id);
    }
}

void Mesh::unbind_textures() const
{
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Mesh::draw_instanced(int32_t const size) const
{
    bind_textures();

    glBindVertexArray(VAO);
    glDrawElementsInstanced(
        GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0, size
    );

    unbind_textures();
}
