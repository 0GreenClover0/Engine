#include "Terrain.h"

#include <iostream>
#include <stb_image.h>

#include "MeshFactory.h"

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
    stbi_set_flip_vertically_on_load(true);

    int32_t width, height, number_of_components;
    unsigned char* data = stbi_load(height_map_path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Height map failed to load at path: " << height_map_path << '\n';
        stbi_image_free(data);
        return MeshFactory::create({}, {}, {}, draw_type, material);
    }

    std::uint32_t texture_id;
    glGenTextures(1, &texture_id);

    GLenum format;
    if (number_of_components == 1)
    {
        format = GL_RED;
    }
    else if (number_of_components == 3)
    {
        format = GL_RGB;
    }
    else if (number_of_components == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cout << "Unknown texture format. Assuming RGBA." << '\n';
        format = GL_RGBA;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    if (format != GL_RGBA)
        std::cout << "Not rgba" << "\n";
    else
        std::cout << "Success" << "\n";

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);

    Texture texture;
    texture.id = texture_id;
    texture.type = TextureType::Heightmap;
    texture.path = height_map_path;

    std::vector<Texture> textures;
    textures.emplace_back(texture);

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
