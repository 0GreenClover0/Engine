#include "Math.h"

#include <glm/ext/quaternion_geometric.hpp>
#include <glm/gtc/epsilon.hpp>

namespace AK
{

glm::vec2 Math::get_normal(glm::vec2 const& v)
{
    glm::vec2 const norm = {-v.y, v.x};
    return glm::normalize(norm);
}

glm::vec2 Math::get_perpendicular_axis(std::array<glm::vec2, 4> const& passed_corners, u8 const index)
{
    u8 const first = index;
    u8 next = index + 1;

    if (index + 1 == 4)
        next = 0;

    return get_normal(passed_corners[next] - passed_corners[first]);
}

glm::vec2 Math::project_on_axis(std::array<glm::vec2, 4> const& vertices, glm::vec2 const& axis)
{
    float min = std::numeric_limits<float>::infinity();
    float max = -std::numeric_limits<float>::infinity();

    for (u8 i = 0; i < 4; ++i)
    {
        float const projection = glm::dot(vertices[i], axis);

        if (projection < min)
        {
            min = projection;
        }

        if (projection > max)
        {
            max = projection;
        }
    }

    return {min, max};
}

bool Math::are_ranges_overlapping(glm::vec2 const& a, glm::vec2 const& b)
{
    // A and B are ranges and it's assumed that a.x <= a.y and b.x <= b.y
    return a.x <= b.y && a.y >= b.x;
}

float Math::get_ranges_overlap_length(glm::vec2 const& a, glm::vec2 const& b)
{
    // A and B are ranges and it's assumed that a.x <= a.y and b.x <= b.y
    if (!are_ranges_overlapping(a, b))
    {
        return 0.f;
    }

    return std::min(a.y, b.y) - std::max(a.x, b.x);
}

bool Math::are_nearly_equal(float const x, float const y, float const epsilon)
{
    return glm::epsilonEqual(x, y, epsilon);
}

glm::vec2 Math::line_intersection(glm::vec2 const& point1, glm::vec2 const& point2, glm::vec2 const& point3, glm::vec2 const& point4)
{
    float const x1 = point1.x, x2 = point2.x, x3 = point3.x, x4 = point4.x;
    float const y1 = point1.y, y2 = point2.y, y3 = point3.y, y4 = point4.y;

    // Calculate the determinant of the coefficient matrix.
    float const det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (glm::abs(det) < 0.001f) // epsilon
    {
        // The lines are parallel (or coincident, if the segments overlap).
        // Shouldn't be vector zero but it won't be used when there's no intersection so whatever.
        return {0.0f, 0.0f};
    }

    // Calculate the x and y coordinates of the intersection point.
    glm::vec2 intersection;
    intersection.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / det;
    intersection.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / det;

    return intersection;
}

}
