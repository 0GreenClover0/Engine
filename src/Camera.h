#pragma once
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <memory>

#include "AK/Badge.h"
#include "Component.h"
#include "Frustum.h"

class Camera final : public Component
{
public:
    static void set_main_camera(std::shared_ptr<Camera> const& camera);

    static std::shared_ptr<Camera> get_main_camera();

    virtual void initialize() override;
    virtual void uninitialize() override;
    virtual void draw_editor() override;

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

    static std::shared_ptr<Camera> create();
    static std::shared_ptr<Camera> create(float const width, float const height, float const fov);

    [[nodiscard]] std::array<glm::vec4, 6> get_frustum_planes();
    [[nodiscard]] glm::mat4 get_view_matrix() const;

    explicit Camera(AK::Badge<Camera>);
    explicit Camera(AK::Badge<Camera>, float const width, float const height, float const fov);

    float width = 1280.0f;
    float height = 720.0f;

    float fov = 1.0f;

    float near_plane = 0.1f;
    float far_plane = 1000.0f;

private:
    void update_internals();
    void update_frustum();

    Frustum m_frustum = {};

    glm::mat4 m_projection = {};

    bool m_dirty = true;

    glm::vec3 m_last_frustum_position = {};

    inline static std::shared_ptr<Camera> m_main_camera;
};
