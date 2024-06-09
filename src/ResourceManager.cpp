#include "ResourceManager.h"

#include "Globals.h"

#include <iostream>
#include <sstream>

#include "MeshFactory.h"
#include "ShaderFactory.h"
#include "TextureLoader.h"

ResourceManager& ResourceManager::get_instance()
{
    static ResourceManager instance;
    return instance;
}

std::shared_ptr<Texture> ResourceManager::load_texture(std::string const& path, TextureType const type, TextureSettings const& settings)
{
    std::string const& key = path; // No need to even call generate_key()
    std::shared_ptr<Texture> resource_ptr = get_from_vector<Texture>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = TextureLoader::get_instance()->load_texture(path, type, settings);
    m_textures.emplace_back(resource_ptr);
    names_to_textures.insert(std::pair<std::string, u16>(key, m_textures.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Texture> ResourceManager::load_cubemap(std::vector<std::string> const& paths, TextureType const type,
                                                       TextureSettings const& settings)
{
    assert(paths.size() >= 6);

    std::stringstream stream;
    stream << paths[0] << paths[1] << paths[2] << paths[3] << paths[4] << paths[5];
    std::string const& key = generate_key(stream);
    std::shared_ptr<Texture> resource_ptr = get_from_vector<Texture>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = TextureLoader::get_instance()->load_cubemap(paths, type, settings);
    m_textures.emplace_back(resource_ptr);
    names_to_textures.insert(std::make_pair(key, m_textures.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Texture> ResourceManager::load_cubemap(std::string const& path, TextureType const type, TextureSettings const& settings)
{
    std::stringstream stream;
    stream << path;
    std::string const& key = generate_key(stream);
    std::shared_ptr<Texture> resource_ptr = get_from_vector<Texture>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = TextureLoader::get_instance()->load_cubemap(path, type, settings);
    m_textures.emplace_back(resource_ptr);
    names_to_textures.insert(std::make_pair(key, m_textures.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Shader> ResourceManager::load_shader(std::string const& compute_path)
{
    std::stringstream stream;
    stream << compute_path;
    std::string const& key = generate_key(stream);
    auto resource_ptr = get_from_vector<Shader>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = ShaderFactory::create(compute_path);
    m_shaders.emplace_back(resource_ptr);
    names_to_shaders.insert(std::make_pair(key, m_shaders.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Shader> ResourceManager::load_shader(std::string const& vertex_path, std::string const& fragment_path)
{
    std::stringstream stream;
    stream << vertex_path << fragment_path;
    std::string const& key = generate_key(stream);

    auto resource_ptr = get_from_vector<Shader>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = ShaderFactory::create(vertex_path, fragment_path);
    m_shaders.emplace_back(resource_ptr);
    names_to_shaders.insert(std::make_pair(key, m_shaders.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Shader> ResourceManager::load_shader(std::string const& vertex_path, std::string const& fragment_path,
                                                     std::string const& geometry_path)
{
    std::stringstream stream;
    stream << vertex_path << fragment_path << geometry_path;
    std::string const& key = generate_key(stream);

    auto resource_ptr = get_from_vector<Shader>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = ShaderFactory::create(vertex_path, fragment_path, geometry_path);
    m_shaders.emplace_back(resource_ptr);
    names_to_shaders.insert(std::make_pair(key, m_shaders.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Shader> ResourceManager::load_shader(std::string const& vertex_path, std::string const& tessellation_control_path,
                                                     std::string const& tessellation_evaluation_path, std::string const& fragment_path)
{
    std::stringstream stream;
    stream << vertex_path << tessellation_control_path << tessellation_evaluation_path << fragment_path;
    std::string const& key = generate_key(stream);

    auto resource_ptr = get_from_vector<Shader>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = ShaderFactory::create(vertex_path, tessellation_control_path, tessellation_evaluation_path, fragment_path);
    m_shaders.emplace_back(resource_ptr);
    names_to_shaders.insert(std::make_pair(key, m_shaders.size() - 1));

    return resource_ptr;
}

std::shared_ptr<Mesh> ResourceManager::load_mesh(u32 const array_id, std::string const& name, std::vector<Vertex> const& vertices,
                                                 std::vector<u32> const& indices, std::vector<std::shared_ptr<Texture>> const& textures,
                                                 DrawType const draw_type, std::shared_ptr<Material> const& material,
                                                 DrawFunctionType const draw_function)
{
    std::stringstream stream;
    stream << name << array_id;

    for (auto const& texture : textures)
    {
        stream << texture->path;
    }

    std::string const& key = generate_key(stream);

    // HACK: We currently don't unload any resources including meshes, even in the editor.
    //       Changing water parameters inside editor recreates the mesh (very similar to changing Sphere's parameters),
    //       which creates lots of big meshes. To work around this we just unload the water mesh everytime someone asks for it.
    if (name == "WATER")
    {
        names_to_meshes.erase(key);
    }

    auto resource_ptr = get_from_vector<Mesh>(key);

    if (resource_ptr != nullptr)
        return resource_ptr;

    resource_ptr = MeshFactory::create(vertices, indices, textures, draw_type, material, draw_function);
    m_meshes.emplace_back(resource_ptr);
    names_to_meshes.insert(std::make_pair(key, m_meshes.size() - 1));

    return resource_ptr;
}

void ResourceManager::reset_state() const
{
    for (auto const& shader : m_shaders)
    {
        shader->materials.clear();
    }

    initialize_default_material();
}

std::string ResourceManager::generate_key(std::stringstream const& stream) const
{
    return stream.str();
}
