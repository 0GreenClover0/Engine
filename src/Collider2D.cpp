#include "Collider2D.h"

#include "DebugDrawing.h"
#include "Engine.h"
#include "Entity.h"
#include "Globals.h"
#include "PhysicsEngine.h"
#include "AK/AK.h"
#include "AK/Math.h"

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
    : m_is_static(is_static), m_collider_type(ColliderType2D::Circle), m_radius(radius)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, glm::vec2 const bounds_dimensions, bool const is_static)
    : m_is_static(is_static), m_collider_type(ColliderType2D::Rectangle), m_width(bounds_dimensions.x), m_height(bounds_dimensions.y)
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

    switch (m_collider_type)
    {
    case ColliderType2D::Circle:
        m_debug_drawing_entity = Debug::draw_debug_sphere({0.0f, 0.0f, 0.0f}, m_radius);
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    case ColliderType2D::Rectangle:
        m_debug_drawing_entity = Debug::draw_debug_box({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {m_width * 2.0f, 0.5f, m_height * 2.0f});
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    default:
        std::unreachable();
    }

    m_debug_drawing = m_debug_drawing_entity->get_component<DebugDrawing>();
    m_debug_drawing->set_radius(m_radius);
    m_debug_drawing->set_extents({m_width * 2.0f, 0.25f, m_height * 2.0f});
}

void Collider2D::uninitialize()
{
    Component::uninitialize();
    PhysicsEngine::get_instance()->remove_collider(std::dynamic_pointer_cast<Collider2D>(shared_from_this()));
}

void Collider2D::awake()
{
    set_can_tick(true);

    // NOTE+FIXME: destroy_immediate might break assumption that entity is not null, it probably should not do that.
    if (entity != nullptr)
    {
        glm::vec2 const position = AK::convert_3d_to_2d(entity->transform->get_position());
        float const angle = glm::radians(entity->transform->get_euler_angles().y);
        compute_axes(position, angle);
    }
}

ColliderType2D Collider2D::get_collider_type() const
{
    return m_collider_type;
}

bool Collider2D::is_trigger() const
{
    return m_is_trigger;
}

void Collider2D::set_is_trigger(bool const is_trigger)
{
    m_is_trigger = is_trigger;
}

bool Collider2D::is_static() const
{
    return m_is_static;
}

void Collider2D::set_is_static(bool const value)
{
    if (m_is_static == value)
    {
        return;
    }

    m_is_static = value;
}

float Collider2D::get_radius_2d() const
{
    return m_radius;
}

glm::vec2 Collider2D::get_center_2d() const
{
    return AK::convert_3d_to_2d(entity->transform->get_position());
}

glm::vec2 Collider2D::get_bounds_dimensions_2d() const
{
    return { m_width, m_height };
}

CollisionInfo Collider2D::overlaps(Collider2D const& other)
{
    CollisionInfo const ci = {};
    bool const any_trigger = m_is_trigger || other.is_trigger();

    // Stupid hack for making both triggers and statics work
    if(any_trigger)
    {
        if (m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Circle)
            return test_collision_circle_circle(*this, other);

        if (m_collider_type == ColliderType2D::Rectangle && other.get_collider_type() == ColliderType2D::Rectangle)
            return test_collision_rectangle_rectangle(*this, other);

        if ((m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Rectangle)
            || (m_collider_type == ColliderType2D::Rectangle && other.get_collider_type() == ColliderType2D::Circle)
            && (m_is_trigger || other.is_trigger()))
            return test_collision_circle_rectangle(*this, other);
    }
    else
    {
        if (m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Circle)
            return test_collision_circle_circle(*this, other);

        if (m_collider_type == ColliderType2D::Rectangle && other.get_collider_type() == ColliderType2D::Rectangle)
            return test_collision_rectangle_rectangle(*this, other);

        if (m_collider_type == ColliderType2D::Circle && other.get_collider_type() == ColliderType2D::Rectangle)
            return test_collision_circle_rectangle(*this, other);
    }

    return ci;
}

void Collider2D::apply_mtv(bool const sign, CollisionInfo const& ci) const
{
    float const factor = sign ? 1.0f : -1.0f;
    glm::vec2 const new_position = AK::convert_3d_to_2d(entity->transform->get_position()) + ci.mtv * 0.5f * factor;
    entity->transform->set_local_position(AK::convert_2d_to_3d(new_position, entity->transform->get_position().y));
}

bool Collider2D::is_inside_trigger(std::string const& guid) const
{
    return m_inside_trigger.contains(guid);
}

std::weak_ptr<Collider2D> Collider2D::get_inside_trigger(std::string const& guid) const
{
    return m_inside_trigger.at(guid);
}

std::vector<std::weak_ptr<Collider2D>> Collider2D::get_inside_trigger_vector() const
{
    return m_inside_trigger_vector;
}

void Collider2D::add_inside_trigger(std::string const& guid, std::shared_ptr<Collider2D> const& collider)
{
    m_inside_trigger.emplace(guid, collider);
    m_inside_trigger_vector.emplace_back(collider);
}

auto Collider2D::set_inside_trigger(std::unordered_map<std::string, std::weak_ptr<Collider2D>> const& map,
    std::vector<std::weak_ptr<Collider2D>> const& vector) -> void
{
    m_inside_trigger = map;
    m_inside_trigger_vector = vector;
}

std::vector<std::weak_ptr<Collider2D>> Collider2D::get_all_overlapping_this_frame() const
{
    return m_overlapped_this_frame;
}

void Collider2D::add_overlapped_this_frame(std::shared_ptr<Collider2D> const& collider)
{
    m_overlapped_this_frame.emplace_back(collider);
    m_overlapped_this_frame_map.emplace(collider->guid, collider);
}

void Collider2D::clear_overlapped_this_frame()
{
    m_overlapped_this_frame.clear();
    m_overlapped_this_frame_map.clear();
}

void Collider2D::update()
{
    // NOTE+FIXME: destroy_immediate might break assumption that entity is not null, it probably should not do that.
    if (entity != nullptr)
    {
        glm::vec2 const position = AK::convert_3d_to_2d(entity->transform->get_position());
        float const angle = glm::radians(entity->transform->get_euler_angles().y);
        compute_axes(position, angle);
    }
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

    m_debug_drawing->set_radius(m_radius);
    m_debug_drawing->set_extents({m_width * 2.0f, 0.25f, m_height * 2.0f});
}

CollisionInfo Collider2D::test_collision_rectangle_rectangle(Collider2D const& obb1, Collider2D const& obb2)
{
    CollisionInfo ci = {};

    std::array const corners1 = { obb1.m_corners[0], obb1.m_corners[1] , obb1.m_corners[2] , obb1.m_corners[3] };
    std::array const corners2 = { obb2.m_corners[0], obb2.m_corners[1] , obb2.m_corners[2] , obb2.m_corners[3] };

    // Get the axes of both rectangles.
    std::array axes1 = { AK::Math::get_perpendicular_axis(corners1, 0), AK::Math::get_perpendicular_axis(corners1, 1) };
    std::array axes2 = { AK::Math::get_perpendicular_axis(corners2, 0), AK::Math::get_perpendicular_axis(corners2, 1) };

    // We need to find the minimal overlap and axis on which it happens.
    float min_overlap = std::numeric_limits<float>::infinity();

    // Check overlap along the axes of both rectangles.
    for (auto& axis : { axes1, axes2 })
    {
        for (u32 i = 0; i < 2; ++i)
        {
            glm::vec2 projection1 = AK::Math::project_on_axis(corners1, axis[i]);
            glm::vec2 projection2 = AK::Math::project_on_axis(corners2, axis[i]);

            float const overlap = AK::Math::get_ranges_overlap_length(projection1, projection2);

            // Shapes are not overlapping
            if (overlap == 0.0f)
            {
                // m_mtv = { 0.0f, 0.0f };
                ci.is_overlapping = false;
                ci.mtv = {0.0f, 0.0f};
                return ci;
            }

            if (overlap < min_overlap)
            {
                min_overlap = overlap;
                ci.mtv = axis[i] * min_overlap;
                ci.is_overlapping = true;
                // m_mtv = axis[i] * min_overlap;
            }
        }
    }

    glm::vec2 const center1 = AK::convert_3d_to_2d(obb1.entity->transform->get_position());
    glm::vec2 const center2 = AK::convert_3d_to_2d(obb2.entity->transform->get_position());

    // Need to reverse MTV if center offset and overlap are not pointing in the same direction.
    if (glm::dot(center1 - center2, ci.mtv) < 0.0f)
        ci.mtv = -ci.mtv;

    return ci;
}

CollisionInfo Collider2D::test_collision_circle_circle(Collider2D const& obb1, Collider2D const& obb2) const
{
    CollisionInfo ci = {};

    glm::vec3 const position1 = obb1.entity->transform->get_position();
    glm::vec3 const position2 = obb2.entity->transform->get_position();

    glm::vec2 const position1_2d = AK::convert_3d_to_2d(position1);
    glm::vec2 const position2_2d = AK::convert_3d_to_2d(position2);

    float const positions_distance = glm::distance(position1_2d, position2_2d);
    float const radius_sum = obb1.get_radius_2d() + obb2.get_radius_2d();

    auto const mtv = 0.5f * glm::normalize(position1_2d - position2_2d) * (get_radius_2d() + obb2.get_radius_2d() - glm::distance(position1_2d, position2_2d));

    if (positions_distance < radius_sum)
    {
        ci.mtv = mtv;
        ci.is_overlapping = true;
    }
    else
    {
        ci.is_overlapping = false;
    }

    return ci;
}

CollisionInfo Collider2D::intersect_circle(glm::vec2 const& center, float const radius, glm::vec2 const& p1, glm::vec2 const& p2)
{
    CollisionInfo ci = {};
    glm::vec2 const v = center - p1;
    glm::vec2 const segment = p2 - p1;
    float const segment_length_squared = glm::dot(segment, segment);

    float t = glm::dot(v, segment) / segment_length_squared;
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec2 const closest_point = p1 + t * segment;
    float const distance = glm::distance(center, closest_point);

    if (distance <= radius)
    {
        ci.mtv = glm::normalize(center - closest_point) * (radius - distance);
        ci.is_overlapping = true;
    }

    return ci;
}

CollisionInfo Collider2D::test_collision_circle_rectangle(Collider2D const& obb1, Collider2D const& obb2)
{
    // Function works in a way that obb1 is always a circle.
    Collider2D const col1 = obb1.get_collider_type() == ColliderType2D::Circle ? obb1 : obb2;
    Collider2D const col2 = obb1.get_collider_type() == ColliderType2D::Rectangle ? obb1 : obb2;

    CollisionInfo ci = {};

    glm::vec2 const center = col1.get_center_2d();
    float const radius = col1.get_radius_2d();
    std::array const corners = { col2.m_corners[0], col2.m_corners[1], col2.m_corners[2], col2.m_corners[3] };
    CollisionInfo collision_info[4] = {};

    glm::vec2 accumulated_mtv = {};
    bool any_overlapped = false;

    // Don't check for intersection if it's a case of circle INSIDE the rectangle. It's handled later.
    if (!is_point_inside_obb(center, corners))
    {
        collision_info[0] = intersect_circle(center, radius, corners[0], corners[1]);
        collision_info[1] = intersect_circle(center, radius, corners[1], corners[2]);
        collision_info[2] = intersect_circle(center, radius, corners[2], corners[3]);
        collision_info[3] = intersect_circle(center, radius, corners[3], corners[0]);
    }

    // Check CollisionInfo of each rectangle, therefore support for intersection between multiple borders in one frame.
    for(u8 i = 0; i < 4; i++)
    {
        if(collision_info[i].is_overlapping)
        {
            any_overlapped = true;

            // Accumulate MTV if colliding with multiple borders, therefore when a corner is inside the circle.
            accumulated_mtv += collision_info[i].mtv;
        }
    }

    // Apply total MTV, might be even two MTVs of borders accumulated when intersecting with a corner.
    if(any_overlapped)
    {
        ci = {true, accumulated_mtv};
        return ci;
    }

    // Check for the case where a circle is inside a rectangle. Therefore, it doesn't intersect with rectangle's
    // sides, but still collides with it.
    if (is_point_inside_obb(center, corners))
    {
        // Calculate MTV, needed when spawning a circle inside a rect or fast movement.
        float const max_rect_length = std::max(col2.m_width, col2.m_height);
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
            new_min_distance_vector = AK::Math::line_intersection(center, cast_point, m_corners[first], m_corners[next]) - center;

            if (glm::length(new_min_distance_vector) < glm::length(min_distance_vector))
                min_distance_vector = new_min_distance_vector;
        }

        ci.is_overlapping = true;

        // Change MTV in case of sphere being inside a rectangle
        if (!col1.m_is_trigger && !col2.m_is_trigger)
        {
            ci.mtv = min_distance_vector + radius;
        }
    }
    return ci;
}
