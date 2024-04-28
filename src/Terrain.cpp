#include "Terrain.h"

#include <iostream>
#include <stb_image.h>

#include "MeshFactory.h"
#include "ResourceManager.h"

std::shared_ptr<Terrain> Terrain::create(std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path)
{
    auto terrain = std::make_shared<Terrain>(AK::Badge<Terrain> {}, material, use_gpu, height_map_path);

    terrain->prepare();

    return terrain;
}

Terrain::Terrain(AK::Badge<Terrain>, std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path)
    : Model(material), m_use_gpu(use_gpu), m_height_map_path(height_map_path)
{
    if (use_gpu)
        m_draw_type = DrawType::Patches;
    else
        m_draw_type = DrawType::TriangleStrip;
}

void Terrain::draw() const
{
    assert(m_meshes.size() == 1);

    if (m_use_gpu)
    {
        m_meshes[0]->draw();
    }
    else
    {
        for (u32 strip = 0; strip < m_strips_count; ++strip)
        {
            m_meshes[0]->draw(m_vertices_per_strip, (void*)(sizeof(u32) * m_vertices_per_strip * strip));
        }
    }
}

void Terrain::prepare()
{
    if (m_height_map_path.empty())
    {
        std::cout << "Terrain currently only supports loading from a height map." << "\n";
    }
    else
    {
        if (m_use_gpu)
            m_meshes.emplace_back(create_terrain_from_height_map_gpu());
        else
            m_meshes.emplace_back(create_terrain_from_height_map());
    }
}

std::shared_ptr<Mesh> Terrain::create_terrain_from_height_map_gpu() const
{
    TextureSettings constexpr texture_settings =
    {
        TextureWrapMode::Repeat,
        TextureWrapMode::Repeat,
        TextureWrapMode::Repeat,
        TextureFiltering::Linear,
        TextureFiltering::Linear,
        TextureFiltering::None,
        true,
        false,
    };

    std::shared_ptr<Texture> const heightmap = ResourceManager::get_instance().load_texture(m_height_map_path, TextureType::Heightmap, texture_settings);

    if (heightmap->id == 0)
    {
        std::cout << "Height map failed to load at path: " << m_height_map_path << '\n';
        return ResourceManager::get_instance().load_mesh(m_meshes.size(), m_height_map_path, {}, {}, {}, m_draw_type, material);
    }

    i32 const width = heightmap->width;
    i32 const height = heightmap->height;

    std::vector<std::shared_ptr<Texture>> textures;
    textures.emplace_back(heightmap);

    u32 constexpr resolution = 20;
    std::vector<Vertex> vertices = {};
    vertices.reserve(resolution * resolution * 4);

    for (u32 i = 0; i <= resolution - 1; ++i)
    {
        for (u32 k = 0; k <= resolution - 1; ++k)
        {
            vertices.emplace_back(
                glm::vec3(
                    -static_cast<float>(width) / 2.0f + static_cast<float>(width) * static_cast<float>(i) / static_cast<float>(resolution),
                    0.0f,
                    -static_cast<float>(height) / 2.0f + static_cast<float>(height) * static_cast<float>(k) / static_cast<float>(resolution)
                ),
                glm::vec3(),
                glm::vec2(
                    static_cast<float>(i) / static_cast<float>(resolution),
                    static_cast<float>(k) / static_cast<float>(resolution)
                )
            );

            vertices.emplace_back(
                glm::vec3(
                    -static_cast<float>(width) / 2.0f + static_cast<float>(width) * (static_cast<float>(i) + 1.0f) / static_cast<float>(resolution),
                    0.0f,
                    -static_cast<float>(height) / 2.0f + static_cast<float>(height) * static_cast<float>(k) / static_cast<float>(resolution)
                ),
                glm::vec3(),
                glm::vec2(
                    (static_cast<float>(i) + 1.0f) / static_cast<float>(resolution),
                    static_cast<float>(k) / static_cast<float>(resolution)
                )
            );

            vertices.emplace_back(
                glm::vec3(
                    -static_cast<float>(width) / 2.0f + static_cast<float>(width) * static_cast<float>(i) / static_cast<float>(resolution),
                    0.0f,
                    -static_cast<float>(height) / 2.0f + static_cast<float>(height) * (static_cast<float>(k) + 1.0f) / static_cast<float>(resolution)
                ),
                glm::vec3(),
                glm::vec2(
                    static_cast<float>(i) / static_cast<float>(resolution),
                    (static_cast<float>(k) + 1.0f) / static_cast<float>(resolution)
                )
            );

            vertices.emplace_back(
                glm::vec3(
                    -static_cast<float>(width) / 2.0f + static_cast<float>(width) * (static_cast<float>(i) + 1.0f) / static_cast<float>(resolution),
                    0.0f,
                    -static_cast<float>(height) / 2.0f + static_cast<float>(height) * (static_cast<float>(k) + 1.0f) / static_cast<float>(resolution)
                ),
                glm::vec3(),
                glm::vec2(
                    (static_cast<float>(i) + 1.0f) / static_cast<float>(resolution),
                    (static_cast<float>(k) + 1.0f) / static_cast<float>(resolution)
                )
            );
        }
    }

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), m_height_map_path, vertices, {}, textures, m_draw_type, material, DrawFunctionType::NotIndexed);
}

std::shared_ptr<Mesh> Terrain::create_terrain_from_height_map()
{
    stbi_set_flip_vertically_on_load(true);

    i32 width, height, number_of_components;
    unsigned char* data = stbi_load(m_height_map_path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Height map failed to load at path: " << m_height_map_path << '\n';
        stbi_image_free(data);
        return ResourceManager::get_instance().load_mesh(m_meshes.size(), m_height_map_path, {}, {}, {}, m_draw_type, material);
    }

    std::vector<Vertex> vertices = {};
    vertices.reserve(height * width);

    float constexpr y_scale = 64.0f / 256.0f;
    float constexpr y_shift = 16.0f;

    for (u32 i = 0; i < height; ++i)
    {
        for (u32 k = 0; k < width; ++k)
        {
            unsigned const char* texel = data + (k + width * i) * number_of_components;

            unsigned char const y = texel[0];

            // NOTE: X and Z are reversed here, to reverse winding order
            vertices.emplace_back(glm::vec3(-width / 2.0f + k, y * y_scale - y_shift, -height / 2.0f + i));
        }
    }

    stbi_image_free(data);

    std::vector<u32> indices = {};
    indices.reserve((height - 1) * width * 2);
    for (u32 i = 0; i < height - 1; ++i)
    {
        for (u32 k = 0; k < width; k++)
        {
            for (u32 m = 0; m < 2; m++)
            {
                indices.emplace_back(k + width * (i + m));
            }
        }
    }

    m_strips_count = height - 1;
    m_vertices_per_strip = width * 2;

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), m_height_map_path, vertices, indices, {}, m_draw_type, material);
}
