#pragma once

#include "TextureLoader.h"

class TextureLoaderGL final : public TextureLoader
{
public:
    static std::shared_ptr<TextureLoaderGL> create();

    std::uint32_t texture_from_file(std::string const& path) override;
};
