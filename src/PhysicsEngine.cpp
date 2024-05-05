#include "PhysicsEngine.h"
#include "Entity.h"
#include "AK/AK.h"

void PhysicsEngine::initialize()
{
    auto const physics_engine = std::make_shared<PhysicsEngine>();
    set_instance(physics_engine);
}

void PhysicsEngine::update_physics()
{
    solve_collisions();
}

void PhysicsEngine::on_collision_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_enter(other);
    }
}

void PhysicsEngine::on_collision_exit(std::shared_ptr<Collider2D> const &collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_exit(other);
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

void PhysicsEngine::solve_collisions() const
{
    // Collision detection
    for (u32 i = 0; i < colliders.size(); i++)
    {
        for (u32 j = 0; j < colliders.size(); j++)
        {
            if (i == j || (colliders[i]->is_static() && colliders[j]->is_static()))
                continue;

            std::shared_ptr<Collider2D> collider1 = colliders[i];
            std::shared_ptr<Collider2D> collider2 = colliders[j];
            ColliderType2D const collider1_type = colliders[i]->get_collider_type();
            ColliderType2D const collider2_type = colliders[j]->get_collider_type();

            // CIRCLE/CIRCLE
            if (collider1_type == collider2_type && collider1_type == ColliderType2D::Circle)
            {
                glm::vec3 const position1 = collider1->entity->transform->get_local_position();
                glm::vec3 const position2 = collider2->entity->transform->get_local_position();

                glm::vec2 const position1_2d = AK::convert_3d_to_2d(position1);
                glm::vec2 const position2_2d = AK::convert_3d_to_2d(position2);

                float const positions_distance = glm::distance(position1_2d, position2_2d);
                float const radius_sum = collider1->get_radius_2d() + collider2->get_radius_2d();

                if (positions_distance < radius_sum)
                {
                    on_collision_enter(collider1, collider2);
                    on_collision_enter(collider2, collider1);

                    if (!collider1->is_static())
                        collider1->separate(position1, position2, collider1->get_radius_2d(), collider2->get_radius_2d());

                    if (!collider2->is_static())
                        collider2->separate(position2, position1, collider2->get_radius_2d(), collider1->get_radius_2d());

                    on_collision_exit(collider1, collider2);
                    on_collision_exit(collider2, collider1);
                }
            }

            // RECTANGLE/RECTANGLE
            else if (collider1_type == collider2_type && collider1_type == ColliderType2D::Rectangle)
            {
                float const left1 = collider1->entity->transform->get_local_position().x - collider1->get_bounds_dimensions_2d().x;
                float const right1 = collider1->entity->transform->get_local_position().x + collider1->get_bounds_dimensions_2d().x;
                float const top1 = collider1->entity->transform->get_local_position().z - collider1->get_bounds_dimensions_2d().y;
                float const bottom1 = collider1->entity->transform->get_local_position().z + collider1->get_bounds_dimensions_2d().y;

                float const left2 = collider2->entity->transform->get_local_position().x - collider2->get_bounds_dimensions_2d().x;
                float const right2 = collider2->entity->transform->get_local_position().x + collider2->get_bounds_dimensions_2d().x;
                float const top2 = collider2->entity->transform->get_local_position().z - collider2->get_bounds_dimensions_2d().y;
                float const bottom2 = collider2->entity->transform->get_local_position().z + collider2->get_bounds_dimensions_2d().y;

                if (right1 - left2 > 0 && right2 - left1 > 0 && bottom1 - top2 > 0 && bottom2 - top1 > 0)
                {
                    on_collision_enter(collider1, collider2);
                    on_collision_enter(collider2, collider1);

                    if (!collider1->is_static())
                        collider1->separate(left1, left2, right1, right2, top1, top2, bottom1, bottom2, false);

                    if (!collider2->is_static())
                        collider2->separate(left1, left2, right1, right2, top1, top2, bottom1, bottom2, true);

                    on_collision_exit(collider1, collider2);
                    on_collision_exit(collider2, collider1);
                }
            }

            // CIRCLE/RECTANGLE
            else if (collider1_type != collider2_type)
            {
                if (collider1->get_collider_type() == ColliderType2D::Rectangle)
                {
                    std::shared_ptr<Collider2D> const buffer = collider1;
                    collider1 = collider2;
                    collider2 = buffer;
                }

                // So from now collider1 is a CIRCLE

                glm::vec2 center_2d = collider1->get_center_2d();
                float const radius_2d = collider1->get_radius_2d();

                float const left = collider2->entity->transform->get_local_position().x - collider2->get_bounds_dimensions_2d().x;
                float const right = collider2->entity->transform->get_local_position().x + collider2->get_bounds_dimensions_2d().x;
                float const top = collider2->entity->transform->get_local_position().z - collider2->get_bounds_dimensions_2d().y;
                float const bottom = collider2->entity->transform->get_local_position().z + collider2->get_bounds_dimensions_2d().y;

                glm::vec2 nearest_point = { glm::clamp(center_2d.x, left, right), glm::clamp(center_2d.y, top, bottom) };

                float const center_to_nearest_point_length = glm::distance(center_2d, nearest_point);

                if (center_to_nearest_point_length < radius_2d)
                {
                    on_collision_enter(collider1, collider2);
                    on_collision_enter(collider2, collider1);

                    if (!collider1->is_static())
                        collider1->separate(center_2d, radius_2d, nearest_point, left, right, top, bottom, false);

                    if (!collider2->is_static())
                        collider2->separate(center_2d, radius_2d, nearest_point, left, right, top, bottom, true);

                    on_collision_exit(collider1, collider2);
                    on_collision_exit(collider2, collider1);
                }
            }
        }
    }
}
