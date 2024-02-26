#pragma once

#include <memory>
#include <vector>

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "Bounds.h"
#include "Shader.h"
#include "AK/Badge.h"
#include "AK/Types.h"

class Drawable;

class Material
{
public:
    static std::shared_ptr<Material> create(std::shared_ptr<Shader> const& shader, i32 const render_order = 0, bool const is_gpu_instanced = false, bool const is_billboard = false);

    explicit Material(AK::Badge<Material>, std::shared_ptr<Shader> const& shader, i32 const render_order, bool const is_gpu_instanced, bool const is_billboard);

    [[nodiscard]] bool has_custom_render_order() const
    {
        return m_render_order != 0;
    }

    [[nodiscard]] i32 get_render_order() const;

    std::shared_ptr<Shader> shader;

    // TODO: Expose properties directly from the shader, somehow.
    glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float specular = 1.0f;
    float shininess = 128.0f;

    u32 sector_count = 5;
    u32 stack_count = 5;
    float radius_multiplier = 2.0f;

    // NOTE: This does not work for gpu instanced meshes, as model matrices are batched together in a single SSBO
    bool needs_view_model = false;

    bool needs_skybox = false;

    bool is_billboard = false;

    // TODO: GPU instancing on one material currently supports only the first mesh that was bound to the material.
    bool is_gpu_instanced = false;

    // NOTE: Only valid if is_gpu_instanced is true
    std::vector<glm::mat4> model_matrices = {};
    std::vector<BoundingBoxShader> bounding_boxes = {};
    std::shared_ptr<Drawable> first_drawable = {};
    std::vector<std::shared_ptr<Drawable>> drawables = {};

private:
    // TODO: Negative render order is currently not supported
    i32 m_render_order = 0;

    friend class SceneSerializer;
};
