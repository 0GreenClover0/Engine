#include "AssetPreloader.h"

#include "Debug.h"

#include <fstream>
#include <sstream>

std::shared_ptr<AssetPreloader> AssetPreloader::create()
{
    return std::make_shared<AssetPreloader>(AK::Badge<AssetPreloader> {});
}

AssetPreloader::AssetPreloader(AK::Badge<AssetPreloader>)
{
}

std::optional<std::string> AssetPreloader::get_text_asset(std::string const& asset_path)
{
    if (!preloaded_text_assets.contains(asset_path))
    {
        return {};
    }

    return preloaded_text_assets.at(asset_path);
}

void AssetPreloader::preload_text_asset(std::string const& asset_path)
{
    if (preloaded_text_assets.contains(asset_path))
    {
        return;
    }

    std::ifstream asset_file(asset_path);

    if (!asset_file.is_open())
    {
        Debug::log("Could not open a scene file: " + asset_path + "\n", DebugType::Error);
        return;
    }

    std::stringstream stream;
    stream << asset_file.rdbuf();
    asset_file.close();

    preloaded_text_assets.emplace(asset_path, stream.str());
}
