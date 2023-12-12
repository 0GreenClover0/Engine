#pragma once

#include <memory>
#include <vector>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Shader.h"

class Drawable;

class Material
{
public:
    explicit Material(std::shared_ptr<Shader> const& shader);
    explicit Material(std::shared_ptr<Shader> const& shader, bool const is_gpu_instanced);

    std::shared_ptr<Shader> shader;

    // TODO: Expose properties directly from the shader, somehow.
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float specular = 1.0f;
    float shininess = 128.0f;

    uint32_t sector_count = 5;
    uint32_t stack_count = 5;
    float radius_multiplier = 2.0f;

    // TODO: GPU instancing on one material currently supports only the first mesh that was bound to the material.
    bool is_gpu_instanced = false;

    // NOTE: Only valid if is_gpu_instanced is true
    std::vector<glm::mat4> model_matrices = {};
    std::shared_ptr<Drawable> first_drawable = {};
    std::vector<std::shared_ptr<Drawable>> drawables = {};

private:
    friend class SceneSerializer;
};
