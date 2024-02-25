#include "TextureLoader.h"

Texture TextureLoader::load_texture(std::string const& path, TextureType type)
{
    return { texture_from_file(path), type, path };
}
