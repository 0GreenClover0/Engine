#include "Sphere.h"

#include "Globals.h"

#include <iostream>
#include <glm/trigonometric.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <utility>

#include "MeshFactory.h"
#include "Model.h"
#include "TextureLoader.h"
#include "Vertex.h"

std::shared_ptr<Sphere> Sphere::create()
{
    auto sphere = std::make_shared<Sphere>(AK::Badge<Sphere> {}, default_material);

    return sphere;
}

std::shared_ptr<Sphere> Sphere::create(float radius, u32 sectors, u32 stacks, std::string const& texture_path,
                                       std::shared_ptr<Material> const &material)
{
    auto sphere = std::make_shared<Sphere>(AK::Badge<Sphere> {}, radius, sectors, stacks, texture_path, material);

    return sphere;
}

Sphere::Sphere(AK::Badge<Sphere>, std::shared_ptr<Material> const& material) : Model(material)
{
    m_draw_type = DrawType::TriangleStrip;
    Sphere::prepare();
}

Sphere::Sphere(AK::Badge<Sphere>, float const radius, u32 const sectors, u32 const stacks, std::string const& texture_path,
               std::shared_ptr<Material> const& material)
    : Model(material), sector_count(sectors), stack_count(stacks), texture_path(std::move(texture_path)), radius(radius)
{
    m_draw_type = DrawType::TriangleStrip;
    Sphere::prepare();
}

std::string Sphere::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Sphere::prepare()
{
    m_meshes.emplace_back(create_sphere());
}

void Sphere::reset()
{
    m_meshes.clear();
}

void Sphere::reprepare()
{
    Sphere::reset();
    Sphere::prepare();
}

std::shared_ptr<Mesh> Sphere::create_sphere() const
{
    auto constexpr PI = glm::pi<float>();
    float const length_inverse = 1.0f / radius;

    std::vector<Vertex> vertices;
    std::vector<u32> indices;
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

        std::vector<Texture> diffuse_maps = { TextureLoader::get_instance()->load_texture(texture_path, TextureType::Diffuse) };
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

        m_material->radius_multiplier = radius;
        m_material->sector_count = sector_count;
        m_material->stack_count = stack_count;

        return MeshFactory::create(vertices, indices, textures, m_draw_type, m_material);
    }

    for (u32 x = 0; x <= stack_count; ++x)
    {
        for (u32 y = 0; y <= sector_count; ++y)
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
    for (u32 y = 0; y < sector_count; ++y)
    {
        if (!odd_row)
        {
            for (u32 x = 0; x <= stack_count; ++x)
            {
                indices.push_back(y * (stack_count + 1) + x);
                indices.push_back((y + 1) * (stack_count + 1) + x);
            }
        }
        else
        {
            for (i32 x = stack_count; x >= 0; --x)
            {
                indices.push_back((y + 1) * (stack_count + 1) + x);
                indices.push_back(y * (stack_count + 1) + x);
            }
        }

        odd_row = !odd_row;
    }

    if (!texture_path.empty())
    {
        std::vector<Texture> diffuse_maps = { TextureLoader::get_instance()->load_texture(texture_path, TextureType::Diffuse) };
        textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
    }

    return MeshFactory::create(vertices, indices, textures, m_draw_type, m_material);
}
