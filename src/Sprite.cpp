#include "Sprite.h"

#include "Globals.h"
#include "MeshFactory.h"
#include "ResourceManager.h"
#include "TextureLoader.h"

std::shared_ptr<Sprite> Sprite::create()
{
    auto sprite = std::make_shared<Sprite>(AK::Badge<Sprite> {}, default_material);
    sprite->prepare();

    return sprite;
}

std::shared_ptr<Sprite> Sprite::create(std::shared_ptr<Material> const& material)
{
    auto sprite = std::make_shared<Sprite>(AK::Badge<Sprite> {}, material);
    sprite->prepare();

    return sprite;
}

std::shared_ptr<Sprite> Sprite::create(std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path)
{
    auto sprite = std::make_shared<Sprite>(AK::Badge<Sprite> {}, material, diffuse_texture_path);
    sprite->prepare();

    return sprite;
}

Sprite::Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material) : Model(material)
{
    m_draw_type = DrawType::Triangles;
}

Sprite::Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path)
    : Model(material), diffuse_texture_path(diffuse_texture_path)
{
    m_draw_type = DrawType::Triangles;
}

void Sprite::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    m_meshes.emplace_back(create_sprite());
}

std::shared_ptr<Mesh> Sprite::create_sprite() const
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

    if (!diffuse_texture_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(diffuse_texture_path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), diffuse_texture_path, vertices, indices, textures, m_draw_type, material);
}
