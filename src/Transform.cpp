#include "Transform.h"

#include <iostream>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "AK.h"
#include "Entity.h"

Transform::Transform(std::shared_ptr<Entity> const& entity) : entity(entity)
{
}

glm::vec3 Transform::get_position()
{
    recompute_model_matrix_if_needed();

    return m_position;
}

glm::quat Transform::get_rotation()
{
    recompute_model_matrix_if_needed();

    return m_rotation;
}

glm::vec3 Transform::get_scale()
{
    recompute_model_matrix_if_needed();

    return m_scale;
}

void Transform::set_local_position(glm::vec3 const& position)
{
    auto const is_position_modified = glm::epsilonNotEqual(position, m_local_position, 0.0001f); 
    if (!is_position_modified.x && !is_position_modified.y && !is_position_modified.z)
    {
        return;
    }

    m_local_position = position;

    if (!m_local_dirty)
    {
        for (auto&& child : children)
        {
            child->m_parent_dirty = true;
            child->needs_bounding_box_adjusting = true;
        }
    }

    m_local_dirty = true;
    needs_bounding_box_adjusting = true;
}

glm::vec3 Transform::get_local_position() const
{
    return m_local_position;
}

void Transform::set_local_scale(glm::vec3 const& scale)
{
    auto const is_scale_modified = glm::epsilonNotEqual(scale, m_local_scale, 0.0001f); 
    if (!is_scale_modified.x && !is_scale_modified.y && !is_scale_modified.z)
    {
        return;
    }

    m_local_scale = scale;

    if (!m_local_dirty)
    {
        for (auto&& child : children)
        {
            child->m_parent_dirty = true;
            child->needs_bounding_box_adjusting = true;
        }
    }

    m_local_dirty = true;
    needs_bounding_box_adjusting = true;
}

glm::vec3 Transform::get_local_scale() const
{
    return m_local_scale;
}

void Transform::set_euler_angles(glm::vec3 const& euler_angles)
{
    auto const is_rotation_modified = glm::epsilonNotEqual(euler_angles, m_euler_angles, 0.0001f); 
    if (!is_rotation_modified.x && !is_rotation_modified.y && !is_rotation_modified.z)
    {
        return;
    }

    this->m_euler_angles = euler_angles;

    if (!m_local_dirty)
    {
        for (auto&& child : children)
        {
            child->m_parent_dirty = true;
            child->needs_bounding_box_adjusting = true;
        }
    }

    m_local_dirty = true;
    needs_bounding_box_adjusting = true;
}

glm::vec3 Transform::get_euler_angles() const
{
    return m_euler_angles;
}

glm::vec3 Transform::get_euler_angles_restricted() const
{
    return { glm::mod(glm::mod(m_euler_angles.x, 360.0f) + 360.0f, 360.0f), glm::mod(glm::mod(m_euler_angles.y, 360.0f) + 360.0f, 360.0f), glm::mod(glm::mod(m_euler_angles.z, 360.0f) + 360.0f, 360.0f) };
}

glm::vec3 Transform::get_forward() const
{
    auto direction = glm::vec3(0.0f, 0.0f, -1.0f);
    auto const euler_angles = get_euler_angles();
    direction = glm::rotateX(direction, glm::radians(euler_angles.x));
    direction = glm::rotateY(direction, glm::radians(euler_angles.y));
    direction = glm::rotateZ(direction, glm::radians(euler_angles.z));
    return direction;
}

glm::mat4 const& Transform::get_model_matrix()
{
    recompute_model_matrix_if_needed();

    return m_model_matrix;
}

// Version for no parent
void Transform::compute_model_matrix()
{
    assert(AK::is_uninitialized(parent));

    assert(m_local_dirty || m_parent_dirty);

    m_model_matrix = get_local_model_matrix();

    m_parent_dirty = false;
}

void Transform::compute_model_matrix(glm::mat4 const& parent_global_model_matrix)
{
    assert(m_local_dirty || m_parent_dirty);

    m_model_matrix = parent_global_model_matrix * get_local_model_matrix();

    m_parent_dirty = false;
}

void Transform::compute_local_model_matrix()
{
    glm::mat4 const transform_x = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 const transform_y = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 const transform_z = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 const rotation_matrix = transform_y * transform_x * transform_z;

    m_local_model_matrix = glm::translate(glm::mat4(1.0f), m_local_position) * rotation_matrix * glm::scale(glm::mat4(1.0f), m_local_scale);
    m_local_dirty = false;
}

glm::mat4 Transform::get_local_model_matrix()
{
    if (!m_local_dirty)
        return m_local_model_matrix;

    compute_local_model_matrix();
    return m_local_model_matrix;
}

void Transform::recompute_model_matrix_if_needed()
{
    if (m_local_dirty || m_parent_dirty)
    {
        if (AK::is_uninitialized(parent))
            compute_model_matrix();
        else
            compute_model_matrix(parent.lock()->get_model_matrix());

        glm::decompose(m_model_matrix, m_scale, m_rotation, m_position, m_skew, m_perpective);
    }
}

void Transform::add_child(std::shared_ptr<Transform> const& transform)
{
    children.emplace_back(transform);
    transform->parent = shared_from_this();
}

void Transform::set_parent(std::shared_ptr<Transform> const& parent)
{
    if (parent == nullptr)
    {
        // TODO: Remove from current parrent
        std::cout << "Setting parent to nullptr might not do what you want it to do." << "\n";
        return;
    }

    parent->add_child(shared_from_this());
    m_local_dirty = true;
    needs_bounding_box_adjusting = true;
}

void Transform::set_parent(std::weak_ptr<Transform> const& parent)
{
    parent.lock()->add_child(shared_from_this());
    m_local_dirty = true;
    needs_bounding_box_adjusting = true;
}
