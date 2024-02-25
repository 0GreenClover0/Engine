#include "TextureLoader.h"

Texture TextureLoader::load_texture(std::string const& path, std::string const& type)
{
    return { texture_from_file(path), type, path };
}
