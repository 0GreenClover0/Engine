#include "Mesh.h"

#include <iostream>
#include <glm/gtc/epsilon.hpp>

#include "Globals.h"
#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"

Mesh::Mesh(std::vector<Vertex> const& vertices, std::vector<u32> const& indices, std::vector<Texture> const& textures,
           DrawType const draw_type, std::shared_ptr<Material> const& material, DrawFunctionType const draw_function)
    : vertices(vertices), indices(indices), textures(textures), material(material), m_draw_type(draw_type), m_draw_function(draw_function)
{
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

        for (u32 i = 0; i < 3; ++i)
        {
            for (u32 k = 0; k < 3; ++k)
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
