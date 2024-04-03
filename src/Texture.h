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
    None,
    Nearest,
    Linear,
};

struct TextureSettings
{
    TextureWrapMode wrap_mode_x = TextureWrapMode::Repeat;
    TextureWrapMode wrap_mode_y = TextureWrapMode::Repeat;
    TextureWrapMode wrap_mode_z = TextureWrapMode::Repeat;
    TextureFiltering filtering_min = TextureFiltering::Linear;
    TextureFiltering filtering_max = TextureFiltering::Linear;
    TextureFiltering filtering_mipmap = TextureFiltering::Linear;
    bool generate_mipmaps = true;
    bool flip_vertically = true;
};

struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

struct Texture
{
    // Only valid in OpenGL
    u32 id = 0;

    u32 width = 0;
    u32 height = 0;
    u32 number_of_components = 0;
    TextureType type = TextureType::None;

    // Only valid in DX11
    ID3D11Texture2D* texture_2d = nullptr;
    ID3D11ShaderResourceView* shader_resource_view = nullptr;
    ID3D11SamplerState* image_sampler_state = nullptr;

    std::string path = {};
};
