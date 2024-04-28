#include "Skybox.h"

#include <iostream>
#include <utility>
#include <glad/glad.h>

#include "Globals.h"
#include "ResourceManager.h"

Skybox::Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths) : Drawable(material), m_face_paths(face_paths)
{
    load_textures();
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
        TextureFiltering::None,
        false,
        false
    };

    m_texture = ResourceManager::get_instance().load_cubemap(m_face_paths, TextureType::None, texture_settings);
}

void Skybox::set_instance(std::shared_ptr<Skybox> const& skybox)
{
    m_instance = skybox;
}

std::shared_ptr<Skybox> Skybox::get_instance()
{
    return m_instance;
}
