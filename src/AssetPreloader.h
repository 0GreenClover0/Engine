#pragma once

#include "AK/Badge.h"

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

class AssetPreloader
{
public:
    static std::shared_ptr<AssetPreloader> create();

    explicit AssetPreloader(AK::Badge<AssetPreloader>);

    std::optional<std::string> get_text_asset(std::string const& asset_path);
    void preload_text_asset(std::string const& asset_path);

    std::unordered_map<std::string, std::string> preloaded_text_assets = {};
};
