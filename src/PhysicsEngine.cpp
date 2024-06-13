#include "PhysicsEngine.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Debug.h"
#include "Engine.h"
#include "Entity.h"

void PhysicsEngine::initialize()
{
    auto const physics_engine = std::make_shared<PhysicsEngine>();
    set_instance(physics_engine);
}

void PhysicsEngine::update_physics() const
{
    for (auto const& collider : colliders)
    {
        collider->physics_update();
    }

    solve_collisions();
}

void PhysicsEngine::on_collision_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_enter(other);
    }
}

void PhysicsEngine::on_collision_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_exit(other);
    }
}

void PhysicsEngine::on_trigger_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_trigger_enter(other);
    }
}

void PhysicsEngine::on_trigger_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_trigger_exit(other);
    }
}

void PhysicsEngine::emplace_collider(std::shared_ptr<Collider2D> const& collider)
{
    colliders.emplace_back(collider);
}

void PhysicsEngine::remove_collider(std::shared_ptr<Collider2D> const& collider)
{
    AK::swap_and_erase(colliders, collider);
}

bool PhysicsEngine::compute_penetration(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other,
                                        glm::vec2& mtv)
{
    assert(collider != nullptr);
    assert(other != nullptr);

    if (collider->collider_type == ColliderType2D::Circle && other->collider_type == ColliderType2D::Circle)
    {
        return test_collision_circle_circle(*collider, *other, mtv);
    }

    if (collider->collider_type == ColliderType2D::Rectangle && other->collider_type == ColliderType2D::Rectangle)
    {
        return test_collision_rectangle_rectangle(*collider, *other, mtv);
    }

    if (collider->collider_type == ColliderType2D::Circle && other->collider_type == ColliderType2D::Rectangle)
    {
        return test_collision_circle_rectangle(*collider, *other, mtv);
    }

    if (collider->collider_type == ColliderType2D::Rectangle && other->collider_type == ColliderType2D::Circle)
    {
        bool const overlapped = test_collision_circle_rectangle(*other, *collider, mtv);
        mtv = -mtv;
        return overlapped;
    }

    return false;
}

void PhysicsEngine::solve_collisions() const
{
    // Collision detection
    for (u32 i = 0; i < colliders.size(); i++)
    {
        for (u32 j = 0; j < colliders.size(); j++)
        {
            if (i == j || (colliders[i]->is_static && colliders[j]->is_static))
                continue;

            std::shared_ptr<Collider2D> collider1 = colliders[i];
            std::shared_ptr<Collider2D> collider2 = colliders[j];

            bool const should_overlap_as_trigger = collider1->is_trigger || collider2->is_trigger;

            glm::vec2 mtv = {};

            if (!compute_penetration(collider1, collider2, mtv))
            {
                continue;
            }

            if (should_overlap_as_trigger)
            {
                collider1->add_overlapped_this_frame(collider2);

#if _DEBUG
                if (collider1->is_inside_trigger(collider2->guid))
                {
                    if (!collider2->is_inside_trigger(collider1->guid))
                    {
                        Debug::log("Colllider2 does not have collider1 inside trigger, but the opposite is true", DebugType::Error);
                    }
                }
                else
                {
                    if (collider2->is_inside_trigger(collider1->guid))
                    {
                        Debug::log("Colllider1 does not have collider2 inside trigger, but the opposite is true", DebugType::Error);
                    }
                }
#endif
            }
            else
            {
                on_collision_enter(collider1, collider2);
                on_collision_enter(collider2, collider1);

                if (!collider1->is_static && !collider2->is_static)
                {
                    collider1->apply_mtv(mtv);
                    collider2->apply_mtv(-mtv);
                }
                else if (collider1->is_static)
                {
                    collider2->apply_mtv(-mtv);
                }
                else if (collider2->is_static)
                {
                    collider1->apply_mtv(mtv);
                }

                on_collision_exit(collider1, collider2);
                on_collision_exit(collider2, collider1);
            }
        }
    }

    for (auto const& collider : colliders)
    {
        std::unordered_map<std::string, std::weak_ptr<Collider2D>> new_inside_trigger = {};
        std::vector<std::weak_ptr<Collider2D>> new_inside_trigger_vector = {};

        for (auto const& other : collider->get_all_overlapping_this_frame())
        {
            if (other.expired())
                continue;

            auto const other_locked = other.lock();

            new_inside_trigger.emplace(other_locked->guid, other);
            new_inside_trigger_vector.emplace_back(other);

            if (!collider->is_inside_trigger(other_locked->guid))
            {
                on_trigger_enter(collider, other_locked);
            }
        }

        for (auto const& other : collider->get_inside_trigger_vector())
        {
            if (other.expired())
                continue;

            auto const other_locked = other.lock();

            if (!new_inside_trigger.contains(other_locked->guid))
            {
                on_trigger_exit(collider, other_locked);
            }
        }

        collider->set_inside_trigger(new_inside_trigger, new_inside_trigger_vector);
        collider->clear_overlapped_this_frame();
    }
}

bool PhysicsEngine::test_collision_rectangle_rectangle(Collider2D const& obb1, Collider2D const& obb2, glm::vec2& mtv)
{
    std::array const corners1 = obb1.get_corners();
    std::array const corners2 = obb2.get_corners();

    // Get the axes of both rectangles.
    std::array const axes1 = {AK::Math::get_perpendicular_axis(corners1, 0), AK::Math::get_perpendicular_axis(corners1, 1)};
    std::array const axes2 = {AK::Math::get_perpendicular_axis(corners2, 0), AK::Math::get_perpendicular_axis(corners2, 1)};

    // We need to find the minimal overlap and axis on which it happens.
    float min_overlap = std::numeric_limits<float>::infinity();
    glm::vec2 smallest_axis = {};

    // Check overlap along the axes of the first rectangle.
    for (auto& axis : axes1)
    {
        glm::vec2 projection1 = AK::Math::project_on_axis(corners1, axis);
        glm::vec2 projection2 = AK::Math::project_on_axis(corners2, axis);

        float const overlap = AK::Math::get_ranges_overlap_length(projection1, projection2);

        // Shapes are not overlapping
        if (AK::Math::are_nearly_equal(overlap, 0.0f, 0.05f))
        {
            return false;
        }

        if (overlap < min_overlap)
        {
            min_overlap = overlap;
            smallest_axis = axis;
        }
    }

    // Check overlap along the axes of the second rectangle.
    for (auto& axis : axes2)
    {
        glm::vec2 projection1 = AK::Math::project_on_axis(corners1, axis);
        glm::vec2 projection2 = AK::Math::project_on_axis(corners2, axis);

        float const overlap = AK::Math::get_ranges_overlap_length(projection1, projection2);

        // Shapes are not overlapping
        if (AK::Math::are_nearly_equal(overlap, 0.0f, 0.05f))
        {
            return false;
        }

        if (overlap < min_overlap)
        {
            min_overlap = overlap;
            smallest_axis = axis;
        }
    }

    mtv = smallest_axis * min_overlap;

    glm::vec2 const center1 = AK::convert_3d_to_2d(obb1.entity->transform->get_position());
    glm::vec2 const center2 = AK::convert_3d_to_2d(obb2.entity->transform->get_position());

    // Need to reverse the MTV if center offset and overlap are not pointing in the same direction.
    if (glm::dot(center2 - center1, mtv) < 0.0f)
        mtv = -mtv;

    return true;
}

bool PhysicsEngine::test_collision_circle_circle(Collider2D const& obb1, Collider2D const& obb2, glm::vec2& mtv)
{
    glm::vec2 const center1_2d = obb1.get_center_2d();
    glm::vec2 const center2_2d = obb2.get_center_2d();

    float const positions_distance = glm::distance(center1_2d, center2_2d);
    float const radius_sum = obb1.get_radius_2d() + obb2.get_radius_2d();

    bool const are_overlapping = positions_distance < radius_sum;

    if (are_overlapping)
    {
        mtv = 0.5f * (glm::normalize(center1_2d - center2_2d) * (obb1.get_radius_2d() + obb2.get_radius_2d() - positions_distance));
    }

    return are_overlapping;
}

bool PhysicsEngine::test_collision_circle_rectangle(Collider2D const& circle_collider, Collider2D const& rect_collider, glm::vec2& mtv)
{
    // Function works in a way that obb1 is always a circle.
    assert(circle_collider.collider_type == ColliderType2D::Circle && rect_collider.collider_type == ColliderType2D::Rectangle);

    glm::vec2 const center = circle_collider.get_center_2d();
    float const radius = circle_collider.get_radius_2d();
    std::array const corners = rect_collider.get_corners();

    // Pretty intuitive: We have collision if any side of the rectangle intersects with a circle.
    if (!is_point_inside_obb(center, corners))
    {
        bool is_overlapping = false;
        glm::vec2 mtv_temp = {};
        is_overlapping |= compute_penetration_circle_segment(center, radius, corners[0], corners[1], mtv_temp);
        is_overlapping |= compute_penetration_circle_segment(center, radius, corners[1], corners[2], mtv_temp);
        is_overlapping |= compute_penetration_circle_segment(center, radius, corners[2], corners[3], mtv_temp);
        is_overlapping |= compute_penetration_circle_segment(center, radius, corners[3], corners[0], mtv_temp);

        if (is_overlapping)
        {
            mtv = mtv_temp;
            return true;
        }
    }
    // Check for the case where a circle is inside a rectangle. Therefore, it doesn't intersect with rectangle's
    // sides, but still collides with it.
    else
    {
        // Calculate MTV, needed when spawning a circle inside a rect or fast movement.
        glm::vec2 const dimensions = rect_collider.get_extents();
        float const max_rect_length = std::max(dimensions.x, dimensions.y);

        auto min_distance_vector = glm::vec2(1.0f);
        auto new_min_distance_vector = glm::vec2(0.0f);
        glm::vec2 cast_point = {};

        auto const axes = rect_collider.get_axes();
        std::array const borders = {axes[0], -axes[1], -axes[0], axes[1]};

        for (u8 i = 0; i < 4; ++i)
        {
            u8 const first = i;
            u8 next = first + 1;

            if (next == 4)
                next = 0;

            cast_point = center + borders[i] * max_rect_length;
            new_min_distance_vector = AK::Math::line_intersection(center, cast_point, corners[first], corners[next]) - center;

            if (glm::length(new_min_distance_vector) < glm::length(min_distance_vector))
                min_distance_vector = new_min_distance_vector;
        }

        mtv = min_distance_vector + radius;

        return true;
    }

    return false;
}

bool PhysicsEngine::compute_penetration_circle_segment(glm::vec2 const& center, float const radius, glm::vec2 const& p1,
                                                       glm::vec2 const& p2, glm::vec2& mtv)
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
        mtv += glm::normalize(center - closest_point) * (radius - distance);
        return distance <= radius;
    }

    return false;
}

bool PhysicsEngine::is_point_inside_obb(glm::vec2 const& point, std::array<glm::vec2, 4> const& rectangle_corners)
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
