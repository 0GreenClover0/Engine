#pragma once

#include <memory>
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_vec4.hpp>

#include "Shader.h"

class Material
{
public:
    explicit Material(std::shared_ptr<Shader> shader);

    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float specular = 1.0f;
    float shininess = 32.0f;

    uint32_t sector_count = 5;
    uint32_t stack_count = 5;
    float radius_multiplier = 2.0f;

    std::shared_ptr<Shader> shader;

private:
    friend class SceneSerializer;
};
