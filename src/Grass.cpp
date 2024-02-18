#include "Grass.h"

#include <glm/gtc/random.hpp>

std::shared_ptr<Grass> Grass::create(std::shared_ptr<Material> const& material, uint32_t const grass_count)
{
    auto grass = std::make_shared<Grass>(AK::Badge<Grass> {}, material, grass_count);
    grass->prepare();

    return grass;
}

std::shared_ptr<Grass> Grass::create(std::shared_ptr<Material> const& material, uint32_t const grass_count,
                                     std::string const& diffuse_texture_path)
{
    auto grass = std::make_shared<Grass>(AK::Badge<Grass> {}, material, grass_count, diffuse_texture_path);
    grass->prepare();

    return grass;
}

Grass::Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material, uint32_t const grass_count) : Model(material), grass_count(grass_count)
{
    draw_type = GL_TRIANGLES;
}

Grass::Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material, uint32_t const grass_count, std::string diffuse_texture_path)
    : Model(material), grass_count(grass_count), diffuse_texture_path(std::move(diffuse_texture_path))
{
    draw_type = GL_TRIANGLES;
}

std::string Grass::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Grass::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    meshes.emplace_back(create_blade());
}

Mesh Grass::create_blade() const
{
    std::vector<Vertex> const vertices =
    {
        { glm::vec3(-1.0f, -1.0f, 0.0f), {}, { 0.0f, 0.0f } }, // bottom left
        { glm::vec3(1.0f, -1.0f, 0.0f), {}, { 1.0f, 0.0f } },  // bottom right
        { glm::vec3(1.0f, 1.0f, 0.0f), {}, { 1.0f, 1.0f } },   // top right
        { glm::vec3(-1.0f, 1.0f, 0.0f), {}, { 0.0f, 1.0f } },  // top left
    };

    std::vector<uint32_t> const indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    std::vector<Texture> textures;

    std::vector<Texture> diffuse_maps = {};
    if (!diffuse_texture_path.empty())
        diffuse_maps.emplace_back(load_texture(diffuse_texture_path, "texture_diffuse"));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return Mesh::create(vertices, indices, textures, draw_type, material);
}
