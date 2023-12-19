#include "Camera.h"

#include <array>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 world_up, double yaw, double pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), speed(default_speed), sensitivity(default_sensitivity)
{
    this->position = position;
    this->world_up = world_up;
    this->yaw = yaw;
    this->pitch = pitch;

    update_camera_vectors();
}

Camera::Camera(float x, float y, float z, float up_x, float up_y, float up_z, double yaw, double pitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), speed(default_speed), sensitivity(default_sensitivity)
{
    this->position = glm::vec3(x, y, z);
    this->world_up = glm::vec3(up_x, up_y, up_z);
    this->yaw = yaw;
    this->pitch = pitch;
    this->update_camera_vectors();
}

// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
void Camera::update_frustum()
{
    glm::mat4 world = projection * get_view_matrix();

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

std::array<glm::vec4, 6> Camera::get_frustum_planes() const
{
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

glm::mat4 Camera::get_view_matrix() const
{
    // TODO: Add dirty flags for all these parameters, and cache this matrix
    return glm::lookAt(position, position + front, up);
}

void Camera::update()
{
    update_camera_vectors();
}

void Camera::update_camera_vectors()
{
    glm::dvec3 front = {};
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    this->front = glm::normalize(front);
    this->right = glm::normalize(glm::cross(this->front, this->world_up));
    this->up = glm::normalize(glm::cross(this->right, this->front));

    update_frustum();
}
