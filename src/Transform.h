#pragma once

#include <functional>
#include <glm/detail/type_quat.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Entity;

// TODO: Make transform a component
class Transform : public std::enable_shared_from_this<Transform>
{
public:
    explicit Transform(std::shared_ptr<Entity> const& entity);

    [[nodiscard]] glm::vec3 get_position();

    [[nodiscard]] glm::quat get_rotation();

    void set_scale(glm::vec3 const& scale);
    [[nodiscard]] glm::vec3 get_scale();

    void set_local_position(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_local_position() const;

    void set_local_scale(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_local_scale() const;

    void set_euler_angles(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_euler_angles() const;

    [[nodiscard]] glm::vec3 get_euler_angles_restricted() const;

    [[nodiscard]] glm::vec3 get_forward();
    [[nodiscard]] glm::vec3 get_right();
    [[nodiscard]] glm::vec3 get_up();

    [[nodiscard]] glm::mat4 const& get_model_matrix();

    void compute_model_matrix();
    void compute_model_matrix(glm::mat4 const& parent_global_model_matrix);

    void set_model_matrix(glm::mat4 const& matrix);

    void compute_local_model_matrix();

    void set_parent(std::shared_ptr<Transform> const& new_parent);

    std::vector<std::shared_ptr<Transform>> children;
    std::weak_ptr<Transform> parent = {};
    std::weak_ptr<Entity> entity = {};

    bool needs_bounding_box_adjusting = true;

protected:
    glm::vec3 m_local_position = {0.0f, 0.0f, 0.0f};
    glm::vec3 m_euler_angles = {0.0f, 0.0f, 0.0f};
    glm::quat m_local_rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 m_local_scale = {1.0f, 1.0f, 1.0f};

    glm::vec3 m_position = {};
    glm::quat m_rotation = {};
    glm::vec3 m_scale = {};

    glm::vec3 m_forward = {};
    glm::vec3 m_right = {};
    glm::vec3 m_up = {};

    glm::vec3 m_skew = {};
    glm::vec4 m_perpective = {};

    glm::mat4 m_model_matrix = glm::mat4(1.0f);
    glm::mat4 m_local_model_matrix = glm::mat4(1.0f);
    bool m_local_dirty = true;
    bool m_parent_dirty = false;

    [[nodiscard]] glm::mat4 get_local_model_matrix();

private:
    void recompute_model_matrix_if_needed();
    void recompute_forward_right_up_if_needed();
    void add_child(std::shared_ptr<Transform> const& transform);
    void remove_child(std::shared_ptr<Transform> const& transform);

    void set_dirty();
    void set_parent_dirty();

    glm::vec3 m_world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_euler_angles_when_caching = glm::vec3(std::nanf("0"), std::nanf("0"), std::nanf("0"));
};
