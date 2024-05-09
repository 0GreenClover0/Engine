#include "TextureLoaderDX11.h"

#include <stb_image.h>
#include <DDSTextureLoader11.h>
#include <d3d11.h>
#include <codecvt>

#include "RendererDX11.h"

std::shared_ptr<TextureLoaderDX11> TextureLoaderDX11::create()
{
    std::shared_ptr<TextureLoaderDX11> texture_loader = std::make_shared<TextureLoaderDX11>();
    set_instance(texture_loader);
    return texture_loader;
}

TextureData TextureLoaderDX11::texture_from_file(std::string const& path, TextureSettings const settings)
{
    auto const device = RendererDX11::get_instance_dx11()->get_device();
    stbi_set_flip_vertically_on_load(settings.flip_vertically);

    i32 image_width;
    i32 image_height;
    i32 image_channels;
    i32 constexpr image_desired_channels = 4;
    u8* image_data = stbi_load(path.c_str(), &image_width, &image_height, &image_channels, image_desired_channels);

    assert(image_data);

    // Originally it was ImageWidth * 4, but if I understand it correctly, it's image width * number of channels
    // "SysMemPitch: The distance (in bytes) from the beginning of one line of a texture to the next line" - via microsoft
    i32 const image_pitch = image_width * image_desired_channels;

    D3D11_TEXTURE2D_DESC image_texture_desc = {};
    image_texture_desc.Width = image_width;
    image_texture_desc.Height = image_height;
    image_texture_desc.MipLevels = 1;
    image_texture_desc.ArraySize = 1;
    image_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    image_texture_desc.SampleDesc.Count = 1;
    image_texture_desc.SampleDesc.Quality = 0;
    image_texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
    image_texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA image_subresource_data = {};
    image_subresource_data.pSysMem = image_data;
    image_subresource_data.SysMemPitch = image_pitch;

    ID3D11Texture2D* image_texture = nullptr;
    HRESULT hr = device->CreateTexture2D(&image_texture_desc, &image_subresource_data, &image_texture);

    assert(SUCCEEDED(hr));

    free(image_data);
    ID3D11ShaderResourceView* texture_resource = nullptr;
    hr = device->CreateShaderResourceView(image_texture, nullptr, &texture_resource);

    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC image_sampler_desc = {};

    image_sampler_desc.Filter = convert_filtering_mode(settings.filtering_min, settings.filtering_max, settings.filtering_mipmap);
    image_sampler_desc.AddressU = convert_wrap_mode(settings.wrap_mode_x);
    image_sampler_desc.AddressV = convert_wrap_mode(settings.wrap_mode_y);
    image_sampler_desc.AddressW = convert_wrap_mode(settings.wrap_mode_z);
    image_sampler_desc.MipLODBias = 0.0f;
    image_sampler_desc.MaxAnisotropy = 1;
    image_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    image_sampler_desc.BorderColor[0] = 1.0f;
    image_sampler_desc.BorderColor[1] = 1.0f;
    image_sampler_desc.BorderColor[2] = 1.0f;
    image_sampler_desc.BorderColor[3] = 1.0f;
    image_sampler_desc.MinLOD = 0;
    image_sampler_desc.MaxLOD = FLT_MAX;

    ID3D11SamplerState* image_sampler_state = nullptr;
    hr = device->CreateSamplerState(&image_sampler_desc, &image_sampler_state);

    assert(SUCCEEDED(hr));

    TextureData texture_data;
    texture_data.id = 0;
    texture_data.texture_2d = image_texture;
    texture_data.shader_resource_view = texture_resource;
    texture_data.image_sampler_state = image_sampler_state;
    texture_data.height = image_height;
    texture_data.width = image_width;
    texture_data.number_of_components = image_desired_channels;
    return texture_data;
}

TextureData TextureLoaderDX11::cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings)
{
    return {};
}

TextureData TextureLoaderDX11::cubemap_from_file(std::string const& path, TextureSettings const settings)
{
    auto const device = RendererDX11::get_instance_dx11()->get_device();

    ID3D11Resource* image_texture = nullptr;
    ID3D11ShaderResourceView* texture_srv = nullptr;

    // Convert from string to widestring, because DDS loader expects it
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter = {};
    std::wstring const path_wstring = converter.from_bytes(path);
    HRESULT hr = DirectX::CreateDDSTextureFromFile(device, path_wstring.c_str(), &image_texture, &texture_srv);
    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC image_sampler_desc = {};
    image_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    image_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    image_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    image_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    image_sampler_desc.MipLODBias = 0.0f;
    image_sampler_desc.MaxAnisotropy = 1;
    image_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    image_sampler_desc.BorderColor[0] = 1.0f;
    image_sampler_desc.BorderColor[1] = 1.0f;
    image_sampler_desc.BorderColor[2] = 1.0f;
    image_sampler_desc.BorderColor[3] = 1.0f;
    image_sampler_desc.MinLOD = 0;
    image_sampler_desc.MaxLOD = FLT_MAX;

    ID3D11SamplerState* image_sampler_state = nullptr;
    hr = device->CreateSamplerState(&image_sampler_desc, &image_sampler_state);
    assert(SUCCEEDED(hr));

    // We do not need texture_2d, nor width/height/number of components
    TextureData texture_data;
    texture_data.id = 0;
    texture_data.texture_2d = nullptr;
    texture_data.shader_resource_view = texture_srv;
    texture_data.image_sampler_state = image_sampler_state;
    texture_data.height = NAN;
    texture_data.width = NAN;
    texture_data.number_of_components = NAN;
    return texture_data;
}

D3D11_TEXTURE_ADDRESS_MODE TextureLoaderDX11::convert_wrap_mode(TextureWrapMode const wrap_mode)
{
    switch (wrap_mode)
    {
    case TextureWrapMode::Repeat:
        return D3D11_TEXTURE_ADDRESS_WRAP;
    case TextureWrapMode::ClampToEdge:
        return D3D11_TEXTURE_ADDRESS_CLAMP;
    case TextureWrapMode::ClampToBorder:
        return D3D11_TEXTURE_ADDRESS_BORDER;
    case TextureWrapMode::MirroredRepeat:
        return D3D11_TEXTURE_ADDRESS_MIRROR;
    default:
        std::unreachable();
    }
}

D3D11_FILTER TextureLoaderDX11::convert_filtering_mode(TextureFiltering const texture_filtering_min,
                                                       TextureFiltering const texture_filtering_mag,
                                                       TextureFiltering const texture_filtering_mipmap)
{
    if (texture_filtering_min == TextureFiltering::Nearest)
    {
        if (texture_filtering_mag == TextureFiltering::Nearest)
        {
            if (texture_filtering_mipmap == TextureFiltering::Nearest)
            {
                return D3D11_FILTER_MIN_MAG_MIP_POINT;
            }

            if (texture_filtering_mipmap == TextureFiltering::Linear)
            {
                return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }

            std::unreachable();
        }

        if (texture_filtering_mag == TextureFiltering::Linear)
        {
            if (texture_filtering_mipmap == TextureFiltering::Nearest)
            {
                return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
            }

            if (texture_filtering_mipmap == TextureFiltering::Linear)
            {
                return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }

            std::unreachable();
        }

        std::unreachable();
    }

    if (texture_filtering_min == TextureFiltering::Linear)
    {
        if (texture_filtering_mag == TextureFiltering::Nearest)
        {
            if (texture_filtering_mipmap == TextureFiltering::Nearest)
            {
                return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
            }

            if (texture_filtering_mipmap == TextureFiltering::Linear)
            {
                return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }

            std::unreachable();
        }

        if (texture_filtering_mag == TextureFiltering::Linear)
        {
            if (texture_filtering_mipmap == TextureFiltering::Nearest)
            {
                return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
            }

            if (texture_filtering_mipmap == TextureFiltering::Linear)
            {
                return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            }

            std::unreachable();
        }

        std::unreachable();
    }

    std::unreachable();
}
