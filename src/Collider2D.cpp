#include "Collider2D.h"

#include "Entity.h"
#include "PhysicsEngine.h"
#include "AK/AK.h"

std::shared_ptr<Collider2D> Collider2D::create()
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, ColliderType2D::Circle, 1.0f, false);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(ColliderType2D const collider_type, float const radius, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, collider_type, radius, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(ColliderType2D const collider_type, glm::vec2 const bounds_dimensions, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, collider_type, bounds_dimensions, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(float const width, float const height, bool const is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, width, height, is_static);
    return collider_2d;
}

Collider2D::Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, float const radius, bool const is_static) : m_radius(radius),
    m_is_static(is_static), m_collider_type(collider_type)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, glm::vec2 const bounds_dimensions, bool const is_static)
    : m_bounds_dimensions(bounds_dimensions), m_is_static(is_static), m_collider_type(collider_type)
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

void Collider2D::separate(glm::vec3 const& center1, glm::vec3 const& center2, float const radius1, float const radius2) const
{
    // This is for circle x circle
    glm::vec2 const center1_2d = AK::convert_3d_to_2d(center1);
    glm::vec2 const center2_2d = AK::convert_3d_to_2d(center2);

    glm::vec2 const separation_vector_2d = glm::normalize(center1_2d - center2_2d) * (radius1 + radius2 - glm::distance(center1_2d, center2_2d));

    glm::vec3 const separation_vector = AK::convert_2d_to_3d(separation_vector_2d);
    glm::vec3 const half_separation_vector = separation_vector * 0.5f;

    glm::vec3 const new_position = entity->transform->get_local_position() + half_separation_vector;
    entity->transform->set_local_position(new_position);
}

void Collider2D::separate(glm::vec2 const center, float const radius, glm::vec2 const nearest_point, float const left, float const right,
                          float const top, float const bottom, bool const negate) const
{
    // This is for rectangle x circle

    float separation_direction = 1;
    if (negate)
        separation_direction = -1;

    glm::vec2 separation_vector_2d = {};

    // Case when the center of the circle is exactly on the border of the rectangle
    // (calculate like for rectangles)
    if (center == nearest_point)
    {
        float const left_separation = center.x - left + radius;
        float const right_separation = right - center.x + radius;
        float const top_separation = center.y - top + radius;
        float const bottom_separation = bottom - center.y + radius;

        left_separation < right_separation ? separation_vector_2d.x = -left_separation : separation_vector_2d.x = right_separation;
        top_separation < bottom_separation ? separation_vector_2d.y = -top_separation : separation_vector_2d.y = bottom_separation;

        if (abs(separation_vector_2d.x) < abs(separation_vector_2d.y))
            separation_vector_2d.y = 0;

        if (abs(separation_vector_2d.x) > abs(separation_vector_2d.y))
            separation_vector_2d.x = 0;
    }
    else // Otherwise calculate like for circles
    {
        separation_vector_2d = normalize(center - nearest_point) * (radius - glm::length(center - nearest_point));
    }

    glm::vec3 const separation_vector = AK::convert_2d_to_3d(separation_vector_2d) * separation_direction;
    glm::vec3 const half_separation_vector = separation_vector * 0.5f;
    glm::vec3 const new_position = entity->transform->get_local_position() + half_separation_vector;
    entity->transform->set_local_position(new_position);
}

void Collider2D::separate(float const left1, float const left2, float const right1, float const right2, float const top1, float const top2,
                          float const bottom1, float const bottom2, bool const negate) const
{
    // This is for rectangle x rectangle

    float const left_separation = right1 - left2;
    float const right_separation = right2 - left1;
    float const top_separation = bottom1 - top2;
    float const bottom_separation = bottom2 - top1;

    float separation_direction = 1;
    if (negate)
        separation_direction = -1;

    glm::vec2 separation_vector_2d = {};

    left_separation < right_separation ? separation_vector_2d.x = -left_separation : separation_vector_2d.x = right_separation;
    top_separation < bottom_separation ? separation_vector_2d.y = -top_separation : separation_vector_2d.y = bottom_separation;

    if (abs(separation_vector_2d.x) < abs(separation_vector_2d.y))
        separation_vector_2d.y = 0;
    else
        separation_vector_2d.x = 0;

    glm::vec3 const separation_vector = AK::convert_2d_to_3d(separation_vector_2d) * separation_direction;
    glm::vec3 const half_separation_vector = separation_vector * 0.5f;
    glm::vec3 const new_position = entity->transform->get_local_position() + half_separation_vector;
    entity->transform->set_local_position(new_position);
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
    return m_bounds_dimensions;
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

bool Collider2D::overlaps(Collider2D const& other) const
{
    return overlaps_one_way(other) && other.overlaps_one_way(*this);
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

bool Collider2D::overlaps_one_way(Collider2D const& other) const
{
    for (u32 a = 0; a < m_axes.size(); ++a)
    {
        float t = glm::dot(other.m_corners[0], m_axes[a]);

        // Find the extent of box 2 on axis a.
        float t_min = t;
        float t_max = t;

        for (u32 c = 1; c < m_corners.size(); ++c)
        {
            t = glm::dot(other.m_corners[c], m_axes[a]); // other.corner[c].dot(axis[a]);

            if (t < t_min)
            {
                t_min = t;
            }
            else if (t > t_max)
            {
                t_max = t;
            }
        }

        // We have to subtract off the origin.

        // See if [t_min, t_max] intersects [0, 1]
        if (t_min > 1 + m_origins[a] || t_max < m_origins[a])
        {
            // There was no intersection along this dimension;
            // the boxes cannot possibly overlap.
            return false;
        }
    }

    // There was no dimension along which there is no intersection.
    // Therefore the boxes overlap.
    return true;
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
        m_axes[a] /= glm::pow(glm::length(m_axes[a]), 2); // axis[a].squared_length();
        m_origins[a] = glm::dot(m_corners[0], m_axes[a]); // corner[0].dot(axis[a]);
    }
}
