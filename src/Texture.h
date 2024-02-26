#pragma once

#include <string>

#include "AK/Types.h"

enum class TextureType
{
    None,
    Diffuse,
    Specular,
    Heightmap,
};

enum class TextureWrapMode
{
    Repeat,
    MirroredRepeat,
    ClampToEdge,
    ClampToBorder
};

enum class TextureFiltering
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

struct TextureSettings
{
    TextureWrapMode wrap_mode_x = TextureWrapMode::Repeat;
    TextureWrapMode wrap_mode_y = TextureWrapMode::Repeat;
    TextureWrapMode wrap_mode_z = TextureWrapMode::Repeat;
    TextureFiltering filtering_min = TextureFiltering::LinearMipmapLinear;
    TextureFiltering filtering_max = TextureFiltering::Linear;
    bool generate_mipmaps = true;
    bool flip_vertically = true;
};

struct Texture
{
    u32 id = 0;
    u32 width = 0;
    u32 height = 0;
    u32 number_of_components = 0;
    TextureType type = TextureType::None;
    std::string path = {};
};
