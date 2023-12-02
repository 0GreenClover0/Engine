#pragma once

#include <memory>
#include <glm/detail/type_vec3.hpp>

#include "Shader.h"

class Material
{
public:
    Material(std::shared_ptr<Shader> shader);

    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
    std::shared_ptr<Shader> shader;

    friend class SceneSerializer;
};
