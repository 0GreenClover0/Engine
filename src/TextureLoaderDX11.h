#pragma once

#include <d3d11.h>

#include "TextureLoader.h"

class TextureLoaderDX11 final : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderDX11> create();

private:
    virtual TextureData texture_from_file(std::string const& path, TextureSettings const settings) override;
    virtual TextureData cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) override;
    virtual TextureData cubemap_from_file(std::string const& path, TextureSettings const settings) override;

    static D3D11_TEXTURE_ADDRESS_MODE convert_wrap_mode(TextureWrapMode const wrap_mode);
    static D3D11_FILTER convert_filtering_mode(TextureFiltering const texture_filtering_min, TextureFiltering const texture_filtering_mag,
                                               TextureFiltering const texture_filtering_mipmap);
};
