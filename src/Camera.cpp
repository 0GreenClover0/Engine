#include "Camera.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"

glm::vec3 Camera::get_position() const
{
    return entity->transform->get_position();
}

glm::mat4 Camera::get_projection()
{
    update_internals();

    return m_projection;
}

float Camera::get_near_plane() const
{
    return near_plane;
}

void Camera::set_near_plane(float const value)
{
    m_dirty = true;
    near_plane = value;
}

float Camera::get_far_plane() const
{
    return far_plane;
}

void Camera::set_far_plane(float const value)
{
    m_dirty = true;
    far_plane = value;
}

void Camera::set_width(float const value)
{
    if (glm::epsilonNotEqual(value, m_width, 0.0001f))
    {
        m_dirty = true;
        m_width = value;
    }
}

void Camera::set_height(float const value)
{
    if (glm::epsilonNotEqual(value, m_height, 0.0001f))
    {
        m_dirty = true;
        m_height = value;
    }
}

void Camera::set_fov(float const value)
{
    m_dirty = true;
    m_fov = value;
}

double Camera::get_yaw() const
{
    return m_yaw;
}

void Camera::set_yaw(double const value)
{
    m_dirty = true;
    m_yaw = value;
}

double Camera::get_pitch() const
{
    return m_pitch;
}

void Camera::set_pitch(double const value)
{
    m_dirty = true;
    m_pitch = value;
}

glm::vec3 Camera::get_front()
{
    update_internals();

    return m_front;
}

glm::vec3 Camera::get_up()
{
    update_internals();

    return m_up;
}

Frustum Camera::get_frustum()
{
    update_internals();

    return frustum;
}

Camera::Camera(glm::vec3 const world_up, double const yaw, double const pitch) : sensitivity(default_sensitivity), m_front(glm::vec3(0.0f, 0.0f, -1.0f))
{
    m_world_up = world_up;
    m_pitch = pitch;
    m_yaw = yaw;
    m_dirty = true;
}

// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
void Camera::update_frustum()
{
    last_frustum_position = get_position();

    glm::mat4 world = m_projection * get_view_matrix();

    auto const right_normal = glm::vec3(world[0][3] - world[0][0], world[1][3] - world[1][0], world[2][3] - world[2][0]);
    float const right_length = glm::length(right_normal);
    frustum.right_plane = Plane(
        right_normal / right_length,
        (world[3][3] - world[3][0]) / right_length
    );

    auto const left_normal = glm::vec3(world[0][3] + world[0][0], world[1][3] + world[1][0], world[2][3] + world[2][0]);
    float const left_length = glm::length(left_normal);
    frustum.left_plane = Plane(
        left_normal / left_length,
        (world[3][3] + world[3][0]) / left_length
    );

    auto const bottom_normal = glm::vec3(world[0][3] + world[0][1], world[1][3] + world[1][1], world[2][3] + world[2][1]);
    auto const bottom_length = glm::length(bottom_normal);
    frustum.bottom_plane = Plane(
        bottom_normal / bottom_length,
        (world[3][3] + world[3][1]) / bottom_length
    );

    auto const top_normal = glm::vec3(world[0][3] - world[0][1], world[1][3] - world[1][1], world[2][3] - world[2][1]);
    auto const top_length = glm::length(top_normal);
    frustum.top_plane = Plane(
        top_normal / top_length,
        (world[3][3] - world[3][1]) / top_length
    );

    auto const far_normal = glm::vec3(world[0][3] - world[0][2], world[1][3] - world[1][2], world[2][3] - world[2][2]);
    auto const far_length = glm::length(far_normal);
    frustum.far_plane = Plane(
        far_normal / far_length,
        (world[3][3] - world[3][2]) / far_length
    );

    auto const near_normal = glm::vec3(world[0][3] + world[0][2], world[1][3] + world[1][2], world[2][3] + world[2][2]);
    auto const near_length = glm::length(near_normal);
    frustum.near_plane = Plane(
        near_normal / near_length,
        (world[3][3] + world[3][2]) / near_length
    );
}

std::array<glm::vec4, 6> Camera::get_frustum_planes()
{
    update_internals();

    return
    {
        glm::vec4(frustum.left_plane.normal, frustum.left_plane.distance),
        glm::vec4(frustum.right_plane.normal, frustum.right_plane.distance),
        glm::vec4(frustum.top_plane.normal, frustum.top_plane.distance),
        glm::vec4(frustum.bottom_plane.normal, frustum.bottom_plane.distance),
        glm::vec4(frustum.near_plane.normal, frustum.near_plane.distance),
        glm::vec4(frustum.far_plane.normal, frustum.far_plane.distance),
    };
}

glm::mat4 Camera::get_view_matrix()
{
    update_internals();

    glm::vec3 const position = get_position();
    return glm::lookAt(position, position + m_front, m_up);
}

void Camera::update_internals()
{
    if (m_dirty)
    {
        m_projection = glm::perspective(m_fov, m_width / m_height, near_plane, far_plane);

        update_camera_vectors();

        update_frustum();
    }
    else if (glm::epsilonEqual(last_frustum_position, get_position(), 0.0001f) != glm::bvec3(true, true, true)) // If we only moved we still need to update frustum
    {
        update_frustum();
    }
}

void Camera::update_camera_vectors()
{
    glm::dvec3 front = {};
    front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    front.y = sin(glm::radians(m_pitch));
    front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    this->m_front = glm::normalize(front);
    this->m_right = glm::normalize(glm::cross(this->m_front, this->m_world_up));
    this->m_up = glm::normalize(glm::cross(this->m_right, this->m_front));

    m_dirty = false;
}
