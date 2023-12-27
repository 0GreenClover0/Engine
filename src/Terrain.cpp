#include "Terrain.h"

#include <stb_image.h>

std::shared_ptr<Terrain> Terrain::create(std::shared_ptr<Material> const& material, std::string const& height_map_path)
{
    auto terrain = std::make_shared<Terrain>(material, height_map_path);

    terrain->prepare();

    return terrain;
}

Terrain::Terrain(std::shared_ptr<Material> const& material, std::string const& height_map_path)
    : Model(material), height_map_path(height_map_path)
{
    draw_type = GL_TRIANGLE_STRIP;
}

std::string Terrain::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Terrain::draw() const
{
    assert(meshes.size() == 1);

    for (uint32_t strip = 0; strip < strips_count; ++strip)
    {
        meshes[0].draw(vertices_per_strip, (void*)(sizeof(uint32_t) * vertices_per_strip * strip));
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
        meshes.emplace_back(create_terrain_from_height_map());
    }
}

Mesh Terrain::create_terrain_from_height_map()
{
    stbi_set_flip_vertically_on_load(true);

    int32_t width, height, number_of_components;
    unsigned char* data = stbi_load(height_map_path.c_str(), &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Height map failed to load at path: " << height_map_path << '\n';
        stbi_image_free(data);
        return Mesh::create({}, {}, {}, draw_type, material);
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

    return Mesh::create(vertices, indices, {}, draw_type, material);
}
