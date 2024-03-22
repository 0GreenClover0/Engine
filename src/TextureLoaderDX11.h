#pragma once
#include "TextureLoader.h"
#include <glad/glad.h>
#include <d3d11.h>
class TextureLoaderDX11 : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderDX11> create();
private:
    ID3D11Texture2D* g_image_texture;
    ID3D11SamplerState* g_image_sampler_state;
    ID3D11ShaderResourceView* g_image_shader_resource_view;
    void clean_up();
    virtual TextureData texture_from_file(std::string const& path, TextureSettings const settings) override;
    virtual TextureData cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) override;

    static GLint convert_wrap_mode(TextureWrapMode const wrap_mode);
    static GLint convert_filtering_mode(TextureFiltering const texture_filtering);
};

