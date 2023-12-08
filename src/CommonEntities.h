#pragma once

#include <memory>

#include "Entity.h"

namespace CommonEntities
{

std::shared_ptr<Entity> create_skybox(std::vector<std::string> const& skybox_texture_paths, std::shared_ptr<Transform> const& parent = nullptr);

std::shared_ptr<Entity> create_directional_light(glm::vec3 const& diffuse, glm::vec3 const& angles, std::shared_ptr<Transform> const& parent = nullptr);
std::shared_ptr<Entity> create_point_light(glm::vec3 const& diffuse, std::shared_ptr<Transform> const& parent = nullptr);
std::shared_ptr<Entity> create_spot_light(glm::vec3 const& diffuse, std::shared_ptr<Transform> const& parent = nullptr);
std::shared_ptr<Entity> create_cube(std::shared_ptr<Material> const& material);
std::shared_ptr<Entity> create_cube(std::string const& diffuse_texture_path, std::shared_ptr<Material> const& material);
std::shared_ptr<Entity> create_cube(std::string const& diffuse_texture_path, std::string const& specular_texture_path, std::shared_ptr<Material> const& material);
}
