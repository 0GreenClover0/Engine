#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Texture.h"

class ResourceManager;

struct TextureData
{
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;
    u32 number_of_components = 0;
    ID3D11Texture2D* texture_2d = nullptr;
    ID3D11ShaderResourceView* shader_resource_view = nullptr;
    ID3D11SamplerState* image_sampler_state = nullptr;
};

class TextureLoader
{
public:
    virtual ~TextureLoader() = default;

    static std::shared_ptr<TextureLoader> get_instance()
    {
        return m_instance;
    }

protected:
    static void set_instance(std::shared_ptr<TextureLoader> const& texture_loader)
    {
        m_instance = texture_loader;
    }

private:
    inline static std::shared_ptr<TextureLoader> m_instance;

    [[nodiscard]] std::shared_ptr<Texture> load_texture(std::string const &path, TextureType const type,
                                                        TextureSettings const &settings = {});
    [[nodiscard]] std::shared_ptr<Texture> load_cubemap(std::vector<std::string> const& paths, TextureType const type,
                                                        TextureSettings const& settings = {});
    [[nodiscard]] std::shared_ptr<Texture> load_cubemap(std::string const& path, TextureType const type,
                                                        TextureSettings const& settings = {});

    TextureData virtual texture_from_file(std::string const& path, TextureSettings const settings) = 0;
    TextureData virtual cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) = 0;
    TextureData virtual cubemap_from_file(std::string const& path, TextureSettings const settings) = 0;

    friend class ResourceManager;
};
