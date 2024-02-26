#include "TextureLoader.h"

#include <cassert>

Texture TextureLoader::load_texture(std::string const& path, TextureType const type, TextureSettings const& settings)
{
    auto const [id, width, height, number_of_components] = texture_from_file(path, settings);
    return { id, width, height, number_of_components , type, path };
}

Texture TextureLoader::load_cubemap(std::vector<std::string> const& paths, TextureType const type,
                                    TextureSettings const& settings)
{
    assert(paths.size() > 0);

    auto const [id, width, height, number_of_components] = cubemap_from_files(paths, settings);
    return { id, width, height, number_of_components , type, paths[0] };
}
