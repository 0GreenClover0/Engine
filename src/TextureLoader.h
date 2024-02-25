#pragma once

#include <memory>
#include <string>

#include "Texture.h"

class TextureLoader
{
public:
    virtual ~TextureLoader() = default;

    Texture load_texture(std::string const& path, TextureType type);

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

    std::uint32_t virtual texture_from_file(std::string const& path) = 0;
};
