#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>

class Entity;

// TODO: Make transform a component
class Transform : public std::enable_shared_from_this<Transform>
{
public:
    explicit Transform(std::shared_ptr<Entity> const& entity);

    [[nodiscard]] glm::vec3 get_position();

    [[nodiscard]] glm::quat get_rotation();

    [[nodiscard]] glm::vec3 get_scale();

    void set_local_position(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_local_position() const;

    void set_local_scale(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_local_scale() const;

    void set_euler_angles(glm::vec3 const&);
    [[nodiscard]] glm::vec3 get_euler_angles() const;

    [[nodiscard]] glm::vec3 get_euler_angles_restricted() const;

    [[nodiscard]] glm::vec3 get_forward() const;

    [[nodiscard]] glm::mat4 const& get_model_matrix();

    void compute_model_matrix();
    void compute_model_matrix(glm::mat4 const&);

    void compute_local_model_matrix();

    void set_parent(std::shared_ptr<Transform> const& parent);
    void set_parent(std::weak_ptr<Transform> const& parent);

    std::vector<std::shared_ptr<Transform>> children;
    std::weak_ptr<Transform> parent = {};
    std::weak_ptr<Entity> entity = {};

    bool needs_bounding_box_adjusting = true;

protected:
    glm::vec3 m_local_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_euler_angles = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_local_scale = { 1.0f, 1.0f, 1.0f };

    glm::vec3 m_position = {};
    glm::quat m_rotation = {};
    glm::vec3 m_scale = {};

    glm::vec3 m_skew = {};
    glm::vec4 m_perpective = {};

    glm::mat4 m_model_matrix = glm::mat4(1.0f);
    glm::mat4 m_local_model_matrix = glm::mat4(1.0f);
    bool m_local_dirty = true;
    bool m_parent_dirty = false;

    [[nodiscard]] glm::mat4 get_local_model_matrix();

private:
    void recompute_model_matrix_if_needed();
    void add_child(std::shared_ptr<Transform> const& transform);
};
