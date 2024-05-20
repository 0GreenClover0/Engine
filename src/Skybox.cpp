#include "Skybox.h"

#include <iostream>
#include <utility>

#include "Globals.h"
#include "Renderer.h"
#include "ResourceManager.h"

Skybox::Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths)
    : Drawable(material), m_face_paths(face_paths)
{
    load_textures();
}

Skybox::Skybox(std::shared_ptr<Material> const& material, std::string const& path) : Drawable(material), m_path(path)
{
    load_textures();
}

void Skybox::initialize()
{
    Drawable::initialize();

    set_instance(static_pointer_cast<Skybox>(shared_from_this()));
}

void Skybox::uninitialize()
{
    Drawable::uninitialize();

    if (m_instance == static_pointer_cast<Skybox>(shared_from_this()))
        m_instance = nullptr;
}

void Skybox::load_textures()
{
    TextureSettings constexpr texture_settings = {TextureWrapMode::ClampToEdge,
                                                  TextureWrapMode::ClampToEdge,
                                                  TextureWrapMode::ClampToEdge,
                                                  TextureFiltering::Linear,
                                                  TextureFiltering::Linear,
                                                  TextureFiltering::None,
                                                  false,
                                                  false};

    if (Renderer::renderer_api == Renderer::RendererApi::DirectX11)
    {
        m_texture = ResourceManager::get_instance().load_cubemap(m_path, TextureType::None, texture_settings);
    }
    else if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        m_texture = ResourceManager::get_instance().load_cubemap(m_face_paths, TextureType::None, texture_settings);
    }
}

void Skybox::set_instance(std::shared_ptr<Skybox> const& skybox)
{
    if (m_instance != nullptr)
        Debug::log("Skybox already exists in the scene, setting instance to the new Skybox.", DebugType::Error);

    m_instance = skybox;
}

std::shared_ptr<Skybox> Skybox::get_instance()
{
    return m_instance;
}
