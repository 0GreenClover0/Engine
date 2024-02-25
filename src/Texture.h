#pragma once

#include <cstdint>
#include <string>

enum class TextureType
{
    None,
    Diffuse,
    Specular,
    Heightmap,
};

struct Texture
{
    std::uint32_t id;
    TextureType type;
    std::string path;
};
