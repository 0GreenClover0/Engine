#pragma once
#include <memory>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Component.h"
#include "Frustum.h"

constexpr double default_yaw = -90.0;
constexpr double default_pitch = 0.0;
constexpr double default_sensitivity = 0.1;

class Camera final : public Component
{
public:
    static void set_main_camera(std::shared_ptr<Camera> const& camera)
    {
        main_camera = camera;
    }

    static std::shared_ptr<Camera> get_main_camera()
    {
        return main_camera;
    }

    glm::vec3 get_position() const;
    glm::mat4 get_projection();

    float get_near_plane() const;
    void set_near_plane(float const value);

    float get_far_plane() const;
    void set_far_plane(float const value);

    void set_width(float const value);
    void set_height(float const value);

    void set_fov(float const value);

    double get_yaw() const;
    void set_yaw(double const value);
    double get_pitch() const;
    void set_pitch(double const value);

    glm::vec3 get_front();
    glm::vec3 get_up();

    Frustum get_frustum();

    double sensitivity;

    explicit Camera(glm::vec3 const world_up = glm::vec3(0.0f, 1.0f, 0.0f), double const yaw = default_yaw, double const pitch = default_pitch);

    [[nodiscard]] std::array<glm::vec4, 6> get_frustum_planes();
    [[nodiscard]] glm::mat4 get_view_matrix();

private:
    void update_internals();
    void update_camera_vectors();
    void update_frustum();

    Frustum frustum = {};

    glm::vec3 m_front = {};
    glm::vec3 m_up = {};
    glm::vec3 m_right = {};
    glm::vec3 m_world_up = {};

    glm::mat4 m_projection = {};

    bool m_dirty = true;

    float m_width;
    float m_height;

    double m_yaw;
    double m_pitch;

    float m_fov;

    float near_plane = 0.1f;
    float far_plane = 1000000.0f;

    glm::vec3 last_frustum_position = {};

    inline static std::shared_ptr<Camera> main_camera;
};