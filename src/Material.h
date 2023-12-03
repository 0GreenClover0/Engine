#pragma once

#include <memory>
#include <glm/detail/type_vec4.hpp>

#include "Shader.h"

class Material
{
public:
    explicit Material(std::shared_ptr<Shader> shader);

    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    std::shared_ptr<Shader> shader;

    friend class SceneSerializer;
};
