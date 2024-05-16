#pragma once

#include <array>

#include <glm/vec2.hpp>

#include "Types.h"

namespace AK
{

class Math
{
public:
    static glm::vec2 get_perpendicular_axis(std::array<glm::vec2, 4> const& passed_corners, u8 const index);
    static glm::vec2 get_normal(glm::vec2 const& v);
    static glm::vec2 project_on_axis(std::array<glm::vec2, 4> const& vertices, glm::vec2 const& axis);

    static float get_ranges_overlap_length(glm::vec2 const& a, glm::vec2 const& b);
    static bool are_ranges_overlapping(glm::vec2 const& a, glm::vec2 const& b);

    static bool are_nearly_equal(float const x, float const y);

    static glm::vec2 line_intersection(glm::vec2 const& point1, glm::vec2 const& point2, glm::vec2 const& point3, const glm::vec2& point4);
};

}
