#pragma once
#include <memory>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "Component.h"
#include "Frustum.h"

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

    glm::vec3 get_front() const;
    glm::vec3 get_up() const;

    Frustum get_frustum();

    explicit Camera();

    [[nodiscard]] std::array<glm::vec4, 6> get_frustum_planes();
    [[nodiscard]] glm::mat4 get_view_matrix() const;

private:
    void update_internals();
    void update_frustum();

    Frustum frustum = {};

    glm::mat4 m_projection = {};

    bool m_dirty = true;

    float m_width;
    float m_height;

    float m_fov;

    float near_plane = 0.1f;
    float far_plane = 1000000.0f;

    glm::vec3 last_frustum_position = {};

    inline static std::shared_ptr<Camera> main_camera;
};