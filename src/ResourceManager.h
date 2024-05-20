#pragma once

#include <unordered_map>
#include <vector>

#include "AK/Types.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Texture.h"

// How ResourceManager works:
//
// 1. Generate key (each function may have its own implementation) and save it in a local variable.
// 2. Call template method get_from_vector() specifying desired <TYPE> and providing the key. It will return either nullptr or a valid resource.
// 3a. If a valid resource is returned by get_from_vector(), you've got your resource!
// 3b. If a nullptr is returned by get_from_vector(), an internal loading function is called and the returned value is added to vector along with key and ID to the map.
class ResourceManager
{
public:
    ResourceManager(ResourceManager const&) = delete;
    void operator=(ResourceManager const&) = delete;
    ~ResourceManager() = default;

    static ResourceManager& get_instance();

    std::shared_ptr<Texture> load_texture(std::string const& path, TextureType const type, TextureSettings const& settings = {});
    std::shared_ptr<Texture> load_cubemap(std::vector<std::string> const& paths, TextureType const type,
                                          TextureSettings const& settings = {});
    std::shared_ptr<Texture> load_cubemap(std::string const& path, TextureType const type, TextureSettings const& settings = {});

    std::shared_ptr<Shader> load_shader(std::string const& compute_path);
    std::shared_ptr<Shader> load_shader(std::string const& vertex_path, std::string const& fragment_path);
    std::shared_ptr<Shader> load_shader(std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path);
    std::shared_ptr<Shader> load_shader(std::string const& vertex_path, std::string const& tessellation_control_path,
                                        std::string const& tessellation_evaluation_path, std::string const& fragment_path);

    std::shared_ptr<Mesh> load_mesh(u32 const array_id, std::string const& name, std::vector<Vertex> const& vertices,
                                    std::vector<u32> const& indices, std::vector<std::shared_ptr<Texture>> const& textures,
                                    DrawType const draw_type, std::shared_ptr<Material> const& material,
                                    DrawFunctionType const draw_function = DrawFunctionType::Indexed);

    void reset_state() const;

private:
    ResourceManager() = default;

    template<typename T>
    std::shared_ptr<T> get_from_vector(std::string const& key)
    {
        i32 id = -1;

        // TODO: This can be automatized by extending EngineHeaderTool.
        // For now it's good enough.
        if constexpr (std::is_same_v<T, Texture>)
        {
            auto const it = names_to_textures.find(key);
            if (it != names_to_textures.end())
            {
                id = it->second;
                return m_textures[id];
            }
        }
        else if constexpr (std::is_same_v<T, Mesh>)
        {
            auto const it = names_to_meshes.find(key);
            if (it != names_to_meshes.end())
            {
                id = it->second;
                return m_meshes[id];
            }
        }
        else if constexpr (std::is_same_v<T, Shader>)
        {
            auto const it = names_to_shaders.find(key);
            if (it != names_to_shaders.end())
            {
                id = it->second;
                return m_shaders[id];
            }
        }

        return nullptr;
    }

    [[nodiscard]] std::string generate_key(std::stringstream const& stream) const;

    // NOTE: RM currently doesn't allow unloadading any resources that were previously loaded.
    std::vector<std::shared_ptr<Texture>> m_textures = {};
    std::vector<std::shared_ptr<Mesh>> m_meshes = {};
    std::vector<std::shared_ptr<Shader>> m_shaders = {};

    // KEYS (usually generated from path and optionally additional data) | INDICES, in a respective vector.
    std::unordered_map<std::string, u16> names_to_textures = {};
    std::unordered_map<std::string, u16> names_to_meshes = {};
    std::unordered_map<std::string, u16> names_to_shaders = {};

    inline static std::shared_ptr<ResourceManager> m_instance;
};
