#pragma once

#include "TextureLoader.h"

#include <d3d11.h>

class TextureLoaderDX11 final : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderDX11> create();

private:
    void clean_up() const;
    virtual TextureData texture_from_file(std::string const& path, TextureSettings const settings) override;
    virtual TextureData cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) override;

    ID3D11Texture2D* g_image_texture = nullptr;
    ID3D11SamplerState* g_image_sampler_state = nullptr;
    ID3D11ShaderResourceView* g_image_shader_resource_view = nullptr;
};

