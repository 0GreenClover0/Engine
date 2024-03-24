#pragma once

#include "TextureLoader.h"

class TextureLoaderDX11 final : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderDX11> create();

private:
    void clean_up() const;
    virtual TextureData texture_from_file(std::string const& path, TextureSettings const settings) override;
    virtual TextureData cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) override;
};
