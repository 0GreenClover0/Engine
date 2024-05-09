#pragma once
#include <memory>

#include "Skybox.h"

class SkyboxFactory
{
public:
    static std::shared_ptr<Skybox> create(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);
    static std::shared_ptr<Skybox> create(std::shared_ptr<Material> const& material, std::string const& path);
    static std::shared_ptr<Skybox> create();
};
