#include "Collider2D.h"

#include "Entity.h"
#include "PhysicsEngine.h"
#include "AK/AK.h"

std::shared_ptr<Collider2D> Collider2D::create()
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, ColliderType2D::Circle, 1.0f);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(ColliderType2D const collider_type, float const radius)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, collider_type, radius);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(ColliderType2D const collider_type, glm::vec2 const bounds_dimensions)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, collider_type, bounds_dimensions);
    return collider_2d;
}

Collider2D::Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, float const radius) : m_radius(radius),
    m_collider_type(collider_type)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, glm::vec2 const bounds_dimensions)
    : m_bounds_dimensions(bounds_dimensions), m_collider_type(collider_type)
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
