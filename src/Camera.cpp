#include "Camera.h"

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
}
