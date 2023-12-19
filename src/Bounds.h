#pragma once

#include <glm/vec3.hpp>

#include "Frustum.h"

struct BoundingBox
{
    glm::vec3 min = {};
    glm::vec3 max = {};

    glm::vec3 center = {};
    glm::vec3 extents = {};

    BoundingBox() = default;

    BoundingBox(glm::vec3 min, glm::vec3 max);

    BoundingBox(glm::vec3 const center, float const x, float const y, float const z);

    inline static constexpr glm::vec3 corner_offsets[] = {
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f, -1.0f,  1.0f),
        glm::vec3(-1.0f,  1.0f, -1.0f),
        glm::vec3(-1.0f,  1.0f,  1.0f),
        glm::vec3( 1.0f, -1.0f, -1.0f),
        glm::vec3( 1.0f, -1.0f,  1.0f),
        glm::vec3( 1.0f,  1.0f, -1.0f),
        glm::vec3( 1.0f,  1.0f,  1.0f)
    };

    [[nodiscard]] static int32_t plane_normal_to_index(glm::vec3 const& normal);

    [[nodiscard]] bool half_plane_test(glm::vec3 const& p, glm::vec3 const& normal, float const offset) const;

    [[nodiscard]] bool is_on_or_forward_plane(Plane const& plane) const;

    [[nodiscard]] bool is_in_frustum(Frustum const& frustum) const;
};

struct BoundingBoxShader
{
    glm::vec3 center = {};
    float padding1;
    glm::vec3 extents = {};
    float padding2;

    BoundingBoxShader() = default;

    explicit BoundingBoxShader(BoundingBox const& bounding_box) : center(bounding_box.center), extents(bounding_box.extents)
    {
    }
};
