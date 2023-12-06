#pragma once

#include <glm/detail/type_vec4.hpp>

#include "Material.h"

class MaterialInstance
{
public:
    explicit MaterialInstance(std::shared_ptr<Material> const& material);

    std::shared_ptr<Material> material;

    // TODO: Expose properties directly from the shader, somehow.
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float specular = 1.0f;
    float shininess = 32.0f;

    uint32_t sector_count = 5;
    uint32_t stack_count = 5;
    float radius_multiplier = 2.0f;
};
