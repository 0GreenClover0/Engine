#include "Transform.h"

#include <glm/ext/matrix_transform.hpp>

#include "ak.h"

// Version for no parent
void Transform::compute_model_matrix()
{
    assert(is_uninitialized(parent));

    if (!m_local_dirty)
        return;

    m_model_matrix = get_local_model_matrix();
}

void Transform::compute_model_matrix(glm::mat4 const& parent_global_model_matrix)
{
    if (!m_local_dirty && !m_parent_dirty)
        return;

    m_model_matrix = parent_global_model_matrix * get_local_model_matrix();
}

void Transform::set_local_position(glm::vec3 const& position)
{
    m_local_position = position;
    m_local_dirty = true;
}

void Transform::set_local_scale(glm::vec3 const& scale)
{
    m_local_scale = scale;
    m_local_dirty = true;
}

void Transform::set_euler_angles(glm::vec3 const& euler_angles)
{
    this->m_euler_angles = euler_angles;
    m_local_dirty = true;
}

glm::mat4 const& Transform::get_model_matrix()
{
    if (m_local_dirty || m_parent_dirty)
    {
        if (is_uninitialized(parent))
            compute_model_matrix();
        else
            compute_model_matrix(parent.lock()->get_model_matrix());
    }

    return m_model_matrix;
}

bool Transform::is_local_dirty() const
{
    return m_local_dirty;
}

bool Transform::is_parent_dirty() const
{
    return m_parent_dirty;
}

glm::vec3 Transform::get_local_position() const
{
    return m_local_position;
}

glm::vec3 Transform::get_local_scale() const
{
    return m_local_scale;
}

glm::vec3 Transform::get_euler_angles() const
{
    return m_euler_angles;
}

void Transform::compute_local_model_matrix()
{
    glm::mat4 const transform_x = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 const transform_y = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.y), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 const transform_z = glm::rotate(glm::mat4(1.0f), glm::radians(m_euler_angles.z), glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 const rotation_matrix = transform_y * transform_x * transform_z;

    m_local_model_matrix = glm::translate(glm::mat4(1.0f), m_local_position) * rotation_matrix * glm::scale(glm::mat4(1.0f), m_local_scale);
    m_local_dirty = false;

    for (auto&& child : children)
    {
        child->m_parent_dirty = true;
    }
}

glm::mat4 Transform::get_local_model_matrix()
{
    if (!m_local_dirty)
        return m_local_model_matrix;

    compute_local_model_matrix();
    return m_local_model_matrix;
}

void Transform::add_child(std::shared_ptr<Transform> const& transform)
{
    children.emplace_back(transform);
    transform->parent = shared_from_this();
}

void Transform::set_parent(std::shared_ptr<Transform> const& parent)
{
    parent->add_child(shared_from_this());
    m_parent_dirty = true;
}

void Transform::set_parent(std::weak_ptr<Transform> const& parent)
{
    parent.lock()->add_child(shared_from_this());
    m_parent_dirty = true;
}
