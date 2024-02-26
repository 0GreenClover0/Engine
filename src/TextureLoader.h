#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Texture.h"

struct TextureData
{
    std::uint32_t id = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t number_of_components = 0;
};

class TextureLoader
{
public:
    virtual ~TextureLoader() = default;

    [[nodiscard]] Texture load_texture(std::string const& path, TextureType const type, TextureSettings const& settings = {});
    [[nodiscard]] Texture load_cubemap(std::vector<std::string> const& paths, TextureType const type, TextureSettings const& settings = {});

    static std::shared_ptr<TextureLoader> get_instance()
    {
        return instance;
    }

protected:
    static void set_instance(std::shared_ptr<TextureLoader> const& texture_loader)
    {
        instance = texture_loader;
    }

private:
    inline static std::shared_ptr<TextureLoader> instance;

    TextureData virtual texture_from_file(std::string const& path, TextureSettings const settings) = 0;
    TextureData virtual cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) = 0;
};
