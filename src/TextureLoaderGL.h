#pragma once

#include <glad/glad.h>

#include "TextureLoader.h"

class TextureLoaderGL final : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderGL> create();

private:
    virtual TextureData texture_from_file(std::string const& path, TextureSettings const settings) override;
    virtual TextureData cubemap_from_files(std::vector<std::string> const& paths, TextureSettings const settings) override;
    virtual TextureData cubemap_from_file(std::string const& path, TextureSettings const settings) override;

    static GLint convert_wrap_mode(TextureWrapMode const wrap_mode);
    static GLint convert_filtering_mode(TextureFiltering const texture_filtering, TextureFiltering const mipmap_filtering);
};
