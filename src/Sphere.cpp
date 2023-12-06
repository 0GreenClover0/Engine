#include "Sphere.h"

#include <iostream>
#include <glm/trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <utility>

#include "Model.h"
#include "Vertex.h"

Sphere::Sphere(float const radius, uint32_t const sectors, uint32_t const stacks, std::string texture_path, std::shared_ptr<MaterialInstance> const& material_instance)
    : Model(material_instance), sector_count(sectors), stack_count(stacks), texture_path(std::move(texture_path)), radius(radius)
{
    draw_type = GL_TRIANGLE_STRIP;
    Sphere::prepare();
}

void Sphere::prepare()
{
    meshes.emplace_back(create_sphere());
}

void Sphere::reset()
{
    meshes.clear();
}

void Sphere::reprepare()
{
    Sphere::reset();
    Sphere::prepare();
}

Mesh Sphere::create_sphere() const
{
    auto constexpr PI = glm::pi<float>();
    float const length_inverse = 1.0f / radius;

    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    if (use_geometry_shader)
    {
        Vertex temp = {};
        temp.position = glm::vec3(0.0f, 0.0f, 0.0f);
        vertices.emplace_back(temp);
        vertices.emplace_back(temp);
        vertices.emplace_back(temp);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);

        std::vector<Texture> diffuse_maps = { load_texture() };
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        material_instance->radius_multiplier = radius;
        material_instance->sector_count = sector_count;
        material_instance->stack_count = stack_count;

        return Mesh::create(vertices, indices, textures, draw_type, material_instance);
    }

    for (uint32_t x = 0; x <= stack_count; ++x)
    {
        for (uint32_t y = 0; y <= sector_count; ++y)
        {
            float const x_segment = static_cast<float>(x) / static_cast<float>(stack_count);
            float const y_segment = static_cast<float>(y) / static_cast<float>(sector_count);

            float const x_position = radius * glm::cos(x_segment * 2.0f * PI) * glm::sin(y_segment * PI);
            float const y_position = radius * glm::cos(y_segment * PI);
            float const z_position = radius * glm::sin(x_segment * 2.0f * PI) * glm::sin(y_segment * PI);

            Vertex vertex = {};
            vertex.position = glm::vec3(x_position, y_position, z_position);
            vertex.normal = glm::vec3(x_position * length_inverse, y_position * length_inverse, z_position * length_inverse);
            vertex.texture_coordinates = glm::vec2(x_segment, y_segment);
            vertices.emplace_back(vertex);
        }
    }

    bool odd_row = false;
    for (uint32_t y = 0; y < sector_count; ++y)
    {
        if (!odd_row)
        {
            for (uint32_t x = 0; x <= stack_count; ++x)
            {
                indices.push_back(y * (stack_count + 1) + x);
                indices.push_back((y + 1) * (stack_count + 1) + x);
            }
        }
        else
        {
            for (int32_t x = stack_count; x >= 0; --x)
            {
                indices.push_back((y + 1) * (stack_count + 1) + x);
                indices.push_back(y * (stack_count + 1) + x);
            }
        }

        odd_row = !odd_row;
    }

    std::vector<Texture> diffuse_maps = { load_texture() };
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return Mesh::create(vertices, indices, textures, draw_type, material_instance);
}

Texture Sphere::load_texture() const
{
    Texture texture;
    texture.id = texture_from_file(texture_path.c_str());
    texture.type = "texture_diffuse";
    texture.path = texture_path;
    return texture;
}
