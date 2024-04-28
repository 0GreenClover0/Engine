#include "Grass.h"

#include "Globals.h"
#include "MeshFactory.h"
#include "ResourceManager.h"
#include "TextureLoader.h"

std::shared_ptr<Grass> Grass::create()
{
    auto grass = std::make_shared<Grass>(AK::Badge<Grass> {}, default_material);
    grass->prepare();

    return grass;
}

std::shared_ptr<Grass> Grass::create(std::shared_ptr<Material> const& material)
{
    auto grass = std::make_shared<Grass>(AK::Badge<Grass> {}, material);
    grass->prepare();

    return grass;
}

std::shared_ptr<Grass> Grass::create(std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path)
{
    auto grass = std::make_shared<Grass>(AK::Badge<Grass> {}, material, diffuse_texture_path);
    grass->prepare();

    return grass;
}

Grass::Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material) : Model(material)
{
    m_draw_type = DrawType::Triangles;
}

Grass::Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path)
    : Model(material), m_diffuse_texture_path(diffuse_texture_path)
{
    m_draw_type = DrawType::Triangles;
}

void Grass::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    m_meshes.emplace_back(create_blade());
}

std::shared_ptr<Mesh> Grass::create_blade() const
{
    std::vector<Vertex> const vertices =
    {
        { glm::vec3(-1.0f, -1.0f, 0.0f), {}, { 0.0f, 0.0f } }, // bottom left
        { glm::vec3(1.0f, -1.0f, 0.0f), {}, { 1.0f, 0.0f } },  // bottom right
        { glm::vec3(1.0f, 1.0f, 0.0f), {}, { 1.0f, 1.0f } },   // top right
        { glm::vec3(-1.0f, 1.0f, 0.0f), {}, { 0.0f, 1.0f } },  // top left
    };

    std::vector<u32> const indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};
    TextureSettings texture_settings = {};
    texture_settings.wrap_mode_x = TextureWrapMode::ClampToEdge;
    texture_settings.wrap_mode_y = TextureWrapMode::ClampToEdge;

    if (!m_diffuse_texture_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(m_diffuse_texture_path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), m_diffuse_texture_path, vertices, indices, textures, m_draw_type, material);
}
