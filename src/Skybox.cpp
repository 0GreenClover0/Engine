#include "Skybox.h"

#include <iostream>
#include <utility>
#include <glad/glad.h>

#include "Globals.h"
#include "TextureLoader.h"

Skybox::Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths) : Drawable(material), m_face_paths(face_paths)
{
    load_textures();
}

std::string Skybox::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Skybox::load_textures()
{
    TextureSettings constexpr texture_settings =
    {
        TextureWrapMode::ClampToEdge,
        TextureWrapMode::ClampToEdge,
        TextureWrapMode::ClampToEdge,
        TextureFiltering::Linear,
        TextureFiltering::Linear,
        false,
        false
    };

    auto const [id, width, height, number_of_components, type, path] = TextureLoader::get_instance()->load_cubemap(m_face_paths, TextureType::None, texture_settings);
    m_texture_id = id;
}
