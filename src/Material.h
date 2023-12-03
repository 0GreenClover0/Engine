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
    glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
    float shininess = 32.0f;

    std::shared_ptr<Shader> shader;

private:
    friend class SceneSerializer;
};
