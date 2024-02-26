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
    std::uint32_t id = 0;
    std::uint32_t width = 0;
    std::uint32_t height = 0;
    std::uint32_t number_of_components = 0;
    TextureType type = TextureType::None;
    std::string path = {};
};
