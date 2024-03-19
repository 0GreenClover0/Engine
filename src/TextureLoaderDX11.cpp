#include "TextureLoaderDX11.h"
#include <stb_image.h>
#include <d3d11.h>
#include <RendererDX11.h>

std::shared_ptr<TextureLoaderDX11> TextureLoaderDX11::create()
{
    std::shared_ptr<TextureLoaderDX11> texture_loader = std::make_shared<TextureLoaderDX11>();
    set_instance(texture_loader);
    return texture_loader;
}

void TextureLoaderDX11::clean_up() const
{
    if (g_image_sampler_state)
        g_image_texture->Release();

    if (g_image_sampler_state)
        g_image_sampler_state->Release();

    if (g_image_shader_resource_view)
        g_image_shader_resource_view->Release();
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
    i32 const image_pitch = image_width * image_channels;

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

    HRESULT hr = device->CreateTexture2D(&image_texture_desc,
                                         &image_subresource_data,
                                         &g_image_texture);

    assert(SUCCEEDED(hr));

    free(image_data);

    hr = device->CreateShaderResourceView(g_image_texture, nullptr, &g_image_shader_resource_view);

    assert(SUCCEEDED(hr));

    D3D11_SAMPLER_DESC image_sampler_desc = {};

    // To use TextureSettings here we would have to write a function to map TextureSettings to DX11's enums, idk if that's necessary for now though
    image_sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    image_sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    image_sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    image_sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    image_sampler_desc.MipLODBias = 0.0f;
    image_sampler_desc.MaxAnisotropy = 1;
    image_sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    image_sampler_desc.BorderColor[0] = 1.0f;
    image_sampler_desc.BorderColor[1] = 1.0f;
    image_sampler_desc.BorderColor[2] = 1.0f;
    image_sampler_desc.BorderColor[3] = 1.0f;
    image_sampler_desc.MinLOD = -FLT_MAX;
    image_sampler_desc.MaxLOD = FLT_MAX;

    hr = device->CreateSamplerState(&image_sampler_desc, &g_image_sampler_state);

    assert(SUCCEEDED(hr));

    TextureData texture_data;
    // TO DO: Figure out how id works in DX11 (apart from the fact that there are samplers registers in the shader)
    texture_data.id = 0;
    texture_data.height = image_height;
    texture_data.width = image_width;
    texture_data.number_of_components = image_desired_channels;
    return texture_data;
}

TextureData TextureLoaderDX11::cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings)
{
    return {};
}
