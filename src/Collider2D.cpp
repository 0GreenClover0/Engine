#include "Collider2D.h"

#include "Engine.h"
#include "Entity.h"
#include "Globals.h"
#include "PhysicsEngine.h"
#include "AK/AK.h"

std::shared_ptr<Collider2D> Collider2D::create()
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, 1.0f, false);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(float const radius, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, radius, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(glm::vec2 const bounds_dimensions, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, bounds_dimensions, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(float const width, float const height, bool const is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, width, height, is_static);
    return collider_2d;
}

bool Collider2D::is_point_inside_obb(glm::vec2 const& point, std::array<glm::vec2, 4> const& rectangle_corners) const
{
    glm::vec2 const ap = point - rectangle_corners[0]; // Vector from one rectangle corner to point
    glm::vec2 const ab = rectangle_corners[1] - rectangle_corners[0]; // One rectangle axis
    glm::vec2 const ad = rectangle_corners[3] - rectangle_corners[0]; // Another rectangle axis

    // Dot products
    float const ap_dot_ab = glm::dot(ap, ab);
    float const ab_dot_ab = glm::dot(ab, ab);
    float const ap_dot_ad = glm::dot(ap, ad);
    float const ad_dot_ad = glm::dot(ad, ad);

    return (0.0f <= ap_dot_ab && ap_dot_ab <= ab_dot_ab) && (0.0f <= ap_dot_ad && ap_dot_ad <= ad_dot_ad);
}

Collider2D::Collider2D(AK::Badge<Collider2D>, float const radius, bool const is_static)
    : m_collider_type(ColliderType2D::Circle), m_is_static(is_static), m_radius(radius)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, glm::vec2 const bounds_dimensions, bool const is_static)
    : m_collider_type(ColliderType2D::Rectangle), m_is_static(is_static), m_width(bounds_dimensions.x), m_height(bounds_dimensions.y)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, float const width, float const height, bool const is_static) : m_is_static(is_static),
    m_collider_type(ColliderType2D::Rectangle), m_width(width), m_height(height)
{
}

void Collider2D::initialize()
{
    Component::initialize();
    PhysicsEngine::get_instance()->emplace_collider(std::dynamic_pointer_cast<Collider2D>(shared_from_this()));
}

void Collider2D::uninitialize()
{
    Component::uninitialize();
    PhysicsEngine::get_instance()->remove_collider(std::dynamic_pointer_cast<Collider2D>(shared_from_this()));
}

void Collider2D::separate(Collider2D const& other) const
{
    // This is for circle x circle
    glm::vec2 const center1_2d = get_center_2d();
    glm::vec2 const center2_2d = other.get_center_2d();

    glm::vec2 const separation_vector_2d = glm::normalize(center1_2d - center2_2d) * 
                                           (get_radius_2d() + other.get_radius_2d() - glm::distance(center1_2d, center2_2d));

    glm::vec3 const separation_vector = AK::convert_2d_to_3d(separation_vector_2d);
    glm::vec3 const half_separation_vector = separation_vector * 0.5f;

    glm::vec3 const new_position = entity->transform->get_local_position() + half_separation_vector;
    entity->transform->set_local_position(new_position);
}

void Collider2D::separate(bool const sign) const
{
    // This is for rectangle x rectangle
    // This is for circle x rectangle (also)
    float const factor = sign ? 1.0f : -1.0f;
    glm::vec2 const new_pos = AK::convert_3d_to_2d(entity->transform->get_position()) + m_mtv * 0.5f * factor;
    entity->transform->set_local_position(AK::convert_2d_to_3d(new_pos, entity->transform->get_position().y));
}

ColliderType2D Collider2D::get_collider_type() const
{
    return m_collider_type;
}

float Collider2D::get_radius_2d() const
{
    return m_radius;
}

glm::vec2 Collider2D::get_center_2d() const
{
    return AK::convert_3d_to_2d(entity->transform->get_local_position());
}

glm::vec2 Collider2D::get_bounds_dimensions_2d() const
{
    return { m_width, m_height };
}

bool Collider2D::is_static() const
{
    return m_is_static;
}

void Collider2D::set_static(bool const value)
{
    if (m_is_static == value)
    {
        return;
    }

    m_is_static = value;
}

bool Collider2D::overlaps(Collider2D& other)
{
    if (!Engine::is_game_running())
        return false;

    if (m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Circle)
        return test_collision_circle_circle(*this, other);

    if (m_collider_type == ColliderType2D::Rectangle && other.get_collider_type() == ColliderType2D::Rectangle)
        return test_collision_rectangle_rectangle(*this, other);

    if (m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Rectangle)
        return test_collision_circle_rectangle(*this, other);

    if (m_collider_type == ColliderType2D::Rectangle && other.get_collider_type() == ColliderType2D::Circle)
        return test_collision_circle_rectangle(other, *this);

    return false;
}

void Collider2D::update()
{
    Component::update();

    // NOTE+FIXME: destroy_immediate might break assumption that entity is not null, it probably should not do that.
    if (entity != nullptr)
    {
        glm::vec2 const position = AK::convert_3d_to_2d(entity->transform->get_position());
        float const angle = glm::radians(entity->transform->get_euler_angles().y);
        compute_axes(position, angle);
    }
}

void Collider2D::start()
{
    Component::start();

    set_can_tick(true);
    compute_axes({ 0.0f, 0.0f }, 0.0f);
}

// NOTE: Should be called everytime the position has changed.
//       Currently we just update it every frame.
void Collider2D::compute_axes(glm::vec2 const& center, float const angle)
{
    // Create a 3D rotation matrix around the y-axis (y-up) and xz-plane.
    glm::mat4 const rotation_matrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Rotate the 2D vectors
    auto const x_3d = glm::vec3(rotation_matrix * glm::vec4(1, 0, 0, 1)); // Rotate the X-axis vector
    auto const z_3d = glm::vec3(rotation_matrix * glm::vec4(0, 0, 1, 1)); // Rotate the Z-axis vector

    // Convert the 3D vectors back to 2D by ignoring the y-component.
    // Not using AK, because we've been rotating individual axes.
    auto x = glm::vec2(x_3d.x, x_3d.z);
    auto y = glm::vec2(z_3d.x, z_3d.z);

    x *= m_width;
    y *= m_height;

    m_corners[0] = center - x - y;
    m_corners[1] = center + x - y;
    m_corners[2] = center + x + y;
    m_corners[3] = center - x + y;

    m_axes[0] = m_corners[1] - m_corners[0];
    m_axes[1] = m_corners[3] - m_corners[0];

    // Make the length of each axis 1/edge length so we know any
    // dot product must be less than 1 to fall within the edge.
    for (u32 a = 0; a < m_axes.size(); ++a)
    {
        m_axes[a] /= glm::pow(glm::length(m_axes[a]), 2);
    }
}

glm::vec2 Collider2D::get_normal(glm::vec2 const& v) const
{
    glm::vec2 const norm = { -v.y, v.x };
    return glm::normalize(norm);
}

glm::vec2 Collider2D::get_perpendicular_axis(std::array<glm::vec2, 4> const& passed_corners, u8 const index) const
{
    u8 const first = index;
    u8 next = index + 1;

    if (index + 1 == 4)
        next = 0;

    return get_normal(passed_corners[next] - passed_corners[first]);
}

glm::vec2 Collider2D::project_on_axis(std::array<glm::vec2, 4> const& vertices, glm::vec2 const& axis) const
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

    return { min, max };
}

glm::vec2 Collider2D::line_intersection(glm::vec2 const& point1, glm::vec2 const& point2, glm::vec2 const& point3,
                                        const glm::vec2& point4) const
{
    float const x1 = point1.x, x2 = point2.x, x3 = point3.x, x4 = point4.x;
    float const y1 = point1.y, y2 = point2.y, y3 = point3.y, y4 = point4.y;

    // Calculate the determinant of the coefficient matrix.
    float const det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (glm::abs(det) < 0.001f) // epsilon
    {
        // The lines are parallel (or coincident, if the segments overlap).
        // Shouldn't be vector zero but it won't be used when there's no intersection so whatever.
        return { 0.0f, 0.0f };
    }

    // Calculate the x and y coordinates of the intersection point.
    glm::vec2 intersection;
    intersection.x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / det;
    intersection.y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / det;

    return intersection;
}

bool are_overlapping(glm::vec2 const& a, glm::vec2 const& b)
{
    // A and B are ranges and it's assumed that a.x <= a.y and b.x <= b.y
    return a.x <= b.y && a.y >= b.x;
}

float get_overlap_length(glm::vec2 const& a, glm::vec2 const& b)
{
    // A and B are ranges and it's assumed that a.x <= a.y and b.x <= b.y
    if (!are_overlapping(a, b))
    {
        return 0.f;
    }

    return std::min(a.y, b.y) - std::max(a.x, b.x);
}

bool Collider2D::test_collision_rectangle_rectangle(Collider2D const& obb1, Collider2D const& obb2)
{
    std::array const corners1 = { obb1.m_corners[0], obb1.m_corners[1] , obb1.m_corners[2] , obb1.m_corners[3] };
    std::array const corners2 = { obb2.m_corners[0], obb2.m_corners[1] , obb2.m_corners[2] , obb2.m_corners[3] };

    // Get the axes of both rectangles.
    std::array axes1 = { get_perpendicular_axis(corners1, 0), get_perpendicular_axis(corners1, 1) };
    std::array axes2 = { get_perpendicular_axis(corners2, 0), get_perpendicular_axis(corners2, 1) };

    // We need to find the minimal overlap and axis on which it happens.
    float min_overlap = std::numeric_limits<float>::infinity();

    // Check overlap along the axes of both rectangles.
    for (auto& axis : { axes1, axes2 })
    {
        for (u32 i = 0; i < 2; ++i)
        {
            glm::vec2 projection1 = project_on_axis(corners1, axis[i]);
            glm::vec2 projection2 = project_on_axis(corners2, axis[i]);

            float const overlap = get_overlap_length(projection1, projection2);

            // Shapes are not overlapping
            if (overlap == 0.0f)
            {
                m_mtv = { 0.0f, 0.0f };
                return false;
            }

            if (overlap < min_overlap)
            {
                min_overlap = overlap;
                m_mtv = axis[i] * min_overlap;
            }
        }
    }

    glm::vec2 const center1 = AK::convert_3d_to_2d(obb1.entity->transform->get_position());
    glm::vec2 const center2 = AK::convert_3d_to_2d(obb2.entity->transform->get_position());

    // Need to reverse MTV if center offset and overlap are not pointing in the same direction.
    if (glm::dot(center1 - center2, m_mtv) < 0.0f)
        m_mtv = -m_mtv;

    return true;
}

bool Collider2D::test_collision_circle_circle(Collider2D const& obb1, Collider2D const& obb2) const
{
    glm::vec3 const position1 = obb1.entity->transform->get_local_position();
    glm::vec3 const position2 = obb2.entity->transform->get_local_position();

    glm::vec2 const position1_2d = AK::convert_3d_to_2d(position1);
    glm::vec2 const position2_2d = AK::convert_3d_to_2d(position2);

    float const positions_distance = glm::distance(position1_2d, position2_2d);
    float const radius_sum = obb1.get_radius_2d() + obb2.get_radius_2d();

    return positions_distance < radius_sum;
}

bool Collider2D::intersect_circle(glm::vec2 const& center, float const radius, glm::vec2 const& p1, glm::vec2 const& p2)
{
    glm::vec2 const v = center - p1;

    glm::vec2 const segment = p2 - p1;
    float const segment_length_squared = glm::dot(segment, segment);

    float t = glm::dot(v, segment) / segment_length_squared;
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec2 const closest_point = p1 + t * segment;
    float const distance = glm::distance(center, closest_point);

    if (distance <= radius)
    {
        m_mtv = glm::normalize(center - closest_point) * (radius - distance);
        return distance <= radius;
    }

    return false;
}

bool Collider2D::test_collision_circle_rectangle(Collider2D& obb1, Collider2D& obb2)
{
    // Function works in a way that obb1 is always a circle.
    glm::vec2 const center = obb1.get_center_2d();
    float const radius = obb1.get_radius_2d();
    std::array const corners = { obb2.m_corners[0], obb2.m_corners[1], obb2.m_corners[2], obb2.m_corners[3] };

    // Pretty intuitive: We have collision if any side of the rectangle intersects with a circle.
    if ((intersect_circle(center, radius, corners[0], corners[1]) || intersect_circle(center, radius, corners[1], corners[2])
         || intersect_circle(center, radius, corners[2], corners[3]) || intersect_circle(center, radius, corners[3], corners[0]))
        && !is_point_inside_obb(center, corners))
    {
        return true;
    }

    // Check for the case where a circle is inside a rectangle. Therefore, it doesn't intersect with rectangle's
    // sides, but still collides with it.
    if (is_point_inside_obb(center, corners))
    {
        // Calculate MTV, needed when spawning a circle inside a rect or fast movement.
        float const max_rect_length = std::max(obb2.m_width, obb2.m_height);
        auto min_distance_vector = glm::vec2(1.0f);
        auto new_min_distance_vector = glm::vec2(0.0f);
        glm::vec2 cast_point = {};

        std::array const borders = { m_axes[0], -m_axes[1], -m_axes[0], m_axes[1] };

        for (u8 i = 0; i < 4; ++i)
        {
            u8 const first = i;
            u8 next = first + 1;

            if (next == 4)
                next = 0;

            cast_point = center + borders[i] * max_rect_length;
            new_min_distance_vector = line_intersection(center, cast_point, m_corners[first], m_corners[next]) - center;

            if (glm::length(new_min_distance_vector) < glm::length(min_distance_vector))
                min_distance_vector = new_min_distance_vector;
        }

        obb1.m_mtv = min_distance_vector + radius;
        obb2.m_mtv = -min_distance_vector - radius;

        return true;
    }

    return false;
}
