#include "Camera.h"

#include <array>
#include <glm/gtc/matrix_transform.hpp>

#include "Entity.h"
#include "Renderer.h"

void Camera::initialize()
{
    Component::initialize();

    Renderer::get_instance()->register_camera(std::dynamic_pointer_cast<Camera>(shared_from_this()));
}

void Camera::uninitialize()
{
    Component::uninitialize();

    if (m_main_camera == shared_from_this())
    {
        m_main_camera = nullptr;
    }

    Renderer::get_instance()->unregister_camera(std::dynamic_pointer_cast<Camera>(shared_from_this()));
}

void Camera::set_main_camera(std::shared_ptr<Camera> const& camera)
{
    m_main_camera = camera;
}

std::shared_ptr<Camera> Camera::get_main_camera()
{
    return m_main_camera;
}

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
    if (glm::epsilonNotEqual(value, width, 0.0001f) && glm::epsilonNotEqual(value, 0.0f, 0.0001f))
    {
        m_dirty = true;
        width = value;
    }
}

void Camera::set_height(float const value)
{
    if (glm::epsilonNotEqual(value, height, 0.0001f) && glm::epsilonNotEqual(value, 0.0f, 0.0001f))
    {
        m_dirty = true;
        height = value;
    }
}

void Camera::set_fov(float const value)
{
    m_dirty = true;
    fov = value;
}

glm::vec3 Camera::get_front() const
{
    return entity->transform->get_forward();
}

glm::vec3 Camera::get_up() const
{
    return entity->transform->get_up();
}

Frustum Camera::get_frustum()
{
    update_internals();

    return m_frustum;
}

std::shared_ptr<Camera> Camera::create()
{
    auto camera = std::make_shared<Camera>(AK::Badge<Camera> {});

    if (get_main_camera() == nullptr)
        set_main_camera(camera);

    return camera;
}

std::shared_ptr<Camera> Camera::create(float const width, float const height, float const fov)
{
    auto camera = std::make_shared<Camera>(AK::Badge<Camera> {}, width, height, fov);

    if (get_main_camera() == nullptr)
        set_main_camera(camera);

    return camera;
}

// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
void Camera::update_frustum()
{
    m_last_frustum_position = get_position();

    glm::mat4 world = m_projection * get_view_matrix();

    auto const right_normal = glm::vec3(world[0][3] - world[0][0], world[1][3] - world[1][0], world[2][3] - world[2][0]);
    float const right_length = glm::length(right_normal);
    m_frustum.right_plane = Plane(right_normal / right_length, (world[3][3] - world[3][0]) / right_length);

    auto const left_normal = glm::vec3(world[0][3] + world[0][0], world[1][3] + world[1][0], world[2][3] + world[2][0]);
    float const left_length = glm::length(left_normal);
    m_frustum.left_plane = Plane(left_normal / left_length, (world[3][3] + world[3][0]) / left_length);

    auto const bottom_normal = glm::vec3(world[0][3] + world[0][1], world[1][3] + world[1][1], world[2][3] + world[2][1]);
    auto const bottom_length = glm::length(bottom_normal);
    m_frustum.bottom_plane = Plane(bottom_normal / bottom_length, (world[3][3] + world[3][1]) / bottom_length);

    auto const top_normal = glm::vec3(world[0][3] - world[0][1], world[1][3] - world[1][1], world[2][3] - world[2][1]);
    auto const top_length = glm::length(top_normal);
    m_frustum.top_plane = Plane(top_normal / top_length, (world[3][3] - world[3][1]) / top_length);

    auto const far_normal = glm::vec3(world[0][3] - world[0][2], world[1][3] - world[1][2], world[2][3] - world[2][2]);
    auto const far_length = glm::length(far_normal);
    m_frustum.far_plane = Plane(far_normal / far_length, (world[3][3] - world[3][2]) / far_length);

    auto const near_normal = glm::vec3(world[0][3] + world[0][2], world[1][3] + world[1][2], world[2][3] + world[2][2]);
    auto const near_length = glm::length(near_normal);
    m_frustum.near_plane = Plane(near_normal / near_length, (world[3][3] + world[3][2]) / near_length);
}

std::array<glm::vec4, 6> Camera::get_frustum_planes()
{
    update_internals();

    return {
        glm::vec4(m_frustum.left_plane.normal, m_frustum.left_plane.distance),
        glm::vec4(m_frustum.right_plane.normal, m_frustum.right_plane.distance),
        glm::vec4(m_frustum.top_plane.normal, m_frustum.top_plane.distance),
        glm::vec4(m_frustum.bottom_plane.normal, m_frustum.bottom_plane.distance),
        glm::vec4(m_frustum.near_plane.normal, m_frustum.near_plane.distance),
        glm::vec4(m_frustum.far_plane.normal, m_frustum.far_plane.distance),
    };
}

glm::mat4 Camera::get_view_matrix() const
{
    glm::vec3 const position = get_position();
    return glm::lookAt(position, position + get_front(), get_up());
}

Camera::Camera(AK::Badge<Camera>)
{
}

Camera::Camera(AK::Badge<Camera>, float const width, float const height, float const fov) : width(width), height(height), fov(fov)
{
}

void Camera::update_internals()
{
    if (m_dirty)
    {
        m_projection = glm::perspective(fov, width / height, near_plane, far_plane);

        update_frustum();
    }
    else if (glm::epsilonEqual(m_last_frustum_position, get_position(), 0.0001f)
             != glm::bvec3(true, true, true)) // If we only moved we still need to update frustum
    {
        update_frustum();
    }
}

#if EDITOR
void Camera::draw_editor()
{
    Component::draw_editor();

    float fov_angle = glm::degrees(get_main_camera()->fov);
    ImGui::InputFloat("Camera FoV", &fov_angle, 1.0f, 90.0f);
    fov = glm::radians(fov_angle);
}
#endif
