#include "Bounds.h"

BoundingBox::BoundingBox(glm::vec3 const min, glm::vec3 const max) : min(min), max(max)
{
    center = (max + min) * 0.5f;
    extents = (max - min) * 0.5f;
}

BoundingBox::BoundingBox(glm::vec3 const center, float const x, float const y, float const z) : center(center), extents(glm::vec3(x, y, z))
{
    max = center + extents * 0.5f;
    min = center - extents * 0.5f;
}

// https://github.com/MichaelBeeu/OctreeDemo/blob/master/src/Frustum.cpp#L33
// http://www.flipcode.com/archives/Octrees_For_Visibility.shtml
i32 BoundingBox::plane_normal_to_index(glm::vec3 const& normal)
{
    i32 index = 0;

    if (normal.z >= 0)
        index |= 1;

    if (normal.y >= 0)
        index |= 2;

    if (normal.x >= 0)
        index |= 4;

    return index;
}

bool BoundingBox::half_plane_test(glm::vec3 const& p, glm::vec3 const& normal, float const offset) const
{
    float const distance = glm::dot(p, normal) + offset;

    // distance < -0.02f Point is behind the plane
    return distance >= -0.02f;
}

bool BoundingBox::is_on_or_forward_plane(Plane const& plane) const
{
    i32 const index = plane_normal_to_index(plane.normal);

    // Test the farthest point of the box from the plane
    // if it's behind the plane, then the entire box will be.
    return half_plane_test(center + extents * corner_offsets[index], plane.normal, plane.distance);
}

bool BoundingBox::is_in_frustum(Frustum const& frustum) const
{
    return is_on_or_forward_plane(frustum.left_plane) && is_on_or_forward_plane(frustum.right_plane)
        && is_on_or_forward_plane(frustum.top_plane) && is_on_or_forward_plane(frustum.bottom_plane)
        && is_on_or_forward_plane(frustum.near_plane) && is_on_or_forward_plane(frustum.far_plane);
}
