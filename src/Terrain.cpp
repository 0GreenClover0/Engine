#include "Terrain.h"

#include <iostream>
#include <stb_image.h>

#include "MeshFactory.h"
#include "TextureLoader.h"

std::shared_ptr<Terrain> Terrain::create(std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path)
{
    auto terrain = std::make_shared<Terrain>(AK::Badge<Terrain> {}, material, use_gpu, height_map_path);

    terrain->prepare();

    return terrain;
}

Terrain::Terrain(AK::Badge<Terrain>, std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path)
    : Model(material), use_gpu(use_gpu), height_map_path(height_map_path)
{
    if (use_gpu)
        draw_type = DrawType::Patches;
    else
        draw_type = DrawType::TriangleStrip;
}

std::string Terrain::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Terrain::draw() const
{
    assert(meshes.size() == 1);

    if (use_gpu)
    {
        meshes[0]->draw();
    }
    else
    {
        for (uint32_t strip = 0; strip < strips_count; ++strip)
        {
            meshes[0]->draw(vertices_per_strip, (void*)(sizeof(uint32_t) * vertices_per_strip * strip));
        }
    }
}

void Terrain::prepare()
{
    if (height_map_path.empty())
    {
        std::cout << "Terrain currently only supports loading from a height map." << "\n";
    }
    else
    {
        if (use_gpu)
            meshes.emplace_back(create_terrain_from_height_map_gpu());
        else
            meshes.emplace_back(create_terrain_from_height_map());
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
        true,
        false,
    };

    Texture const heightmap = TextureLoader::get_instance()->load_texture(height_map_path, TextureType::Heightmap, texture_settings);

    if (heightmap.id == 0)
    {
        std::cout << "Height map failed to load at path: " << height_map_path << '\n';
        return MeshFactory::create({}, {}, {}, draw_type, material);
    }

    int32_t const width = heightmap.width;
    int32_t const height = heightmap.height;

    std::vector<Texture> textures;
    textures.emplace_back(heightmap);

    uint32_t constexpr resolution = 20;
    std::vector<Vertex> vertices = {};
    vertices.reserve(resolution * resolution * 4);

    for (uint32_t i = 0; i <= resolution - 1; ++i)
    {
        for (uint32_t k = 0; k <= resolution - 1; ++k)
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

    return MeshFactory::create(vertices, {}, textures, draw_type, material, DrawFunctionType::NotIndexed);
}

std::shared_ptr<Mesh> Terrain::create_terrain_from_height_map()
{
    stbi_set_flip_vertically_on_load(true);

    int32_t width, height, number_of_components;
    unsigned char* data = stbi_load(height_map_path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Height map failed to load at path: " << height_map_path << '\n';
        stbi_image_free(data);
        return MeshFactory::create({}, {}, {}, draw_type, material);
    }

    std::vector<Vertex> vertices = {};
    vertices.reserve(height * width);

    float constexpr y_scale = 64.0f / 256.0f;
    float constexpr y_shift = 16.0f;

    for (uint32_t i = 0; i < height; ++i)
    {
        for (uint32_t k = 0; k < width; ++k)
        {
            unsigned const char* texel = data + (k + width * i) * number_of_components;

            unsigned char const y = texel[0];

            // NOTE: X and Z are reversed here, to reverse winding order
            vertices.emplace_back(glm::vec3(-width / 2.0f + k, y * y_scale - y_shift, -height / 2.0f + i));
        }
    }

    stbi_image_free(data);

    std::vector<uint32_t> indices = {};
    indices.reserve((height - 1) * width * 2);
    for (uint32_t i = 0; i < height - 1; ++i)
    {
        for (uint32_t k = 0; k < width; k++)
        {
            for (uint32_t m = 0; m < 2; m++)
            {
                indices.emplace_back(k + width * (i + m));
            }
        }
    }

    strips_count = height - 1;
    vertices_per_strip = width * 2;

    return MeshFactory::create(vertices, indices, {}, draw_type, material);
}
