#pragma once

#include "AK/AK.h"
#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"
#include "glm/glm.hpp"

#include <array>
#include <unordered_map>

class DebugDrawing;

struct CollisionInfo
{
    bool is_overlapping = false;
    glm::vec2 mtv = {0, 0};
};

enum class ColliderType2D
{
    Rectangle = 0,
    Circle = 1,
};

class Collider2D final : public Component
{
public:
    static std::shared_ptr<Collider2D> create();
    static std::shared_ptr<Collider2D> create(float const radius, bool const is_static = false);
    static std::shared_ptr<Collider2D> create(glm::vec2 const bounds_dimensions, bool const is_static = false);
    static std::shared_ptr<Collider2D> create(float const width, float const height, bool const is_static = false);

    bool is_point_inside_obb(glm::vec2 const& point, std::array<glm::vec2, 4> const& rectangle_corners) const;

    // CircleCollision
    explicit Collider2D(AK::Badge<Collider2D>, float const radius, bool const is_static);

    // RectangleCollision
    explicit Collider2D(AK::Badge<Collider2D>, glm::vec2 const bounds_dimensions, bool const is_static);
    explicit Collider2D(AK::Badge<Collider2D>, float const width, float const height, bool const is_static);

#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void awake() override;
    void physics_update();

    void add_force(glm::vec2 const force);

    void apply_mtv(glm::vec2 const mtv) const;

    void set_collider_type(ColliderType2D const new_collider_type);

    void set_radius_2d(float const new_radius);
    float get_radius_2d() const;

    void set_extents(glm::vec2 const extents);
    glm::vec2 get_extents() const;

    glm::vec2 get_center_2d() const;

    glm::vec2 get_bounds_dimensions_2d() const;
    void set_bounds_dimensions_2d(float const new_width, float const new_height);

    std::array<glm::vec2, 4> get_corners() const;
    std::array<glm::vec2, 2> get_axes() const;

    // Internal functions meant to be used by the PhysicsEngine
    bool is_inside_trigger(std::string const& guid) const;
    std::weak_ptr<Collider2D> get_inside_trigger(std::string const& guid) const;
    std::vector<std::weak_ptr<Collider2D>> get_inside_trigger_vector() const;
    void add_inside_trigger(std::string const& guid, std::shared_ptr<Collider2D> const& collider);
    void set_inside_trigger(std::unordered_map<std::string, std::weak_ptr<Collider2D>> const& map,
                            std::vector<std::weak_ptr<Collider2D>> const& vector);

    std::vector<std::weak_ptr<Collider2D>> get_all_overlapping_this_frame() const;
    void add_overlapped_this_frame(std::shared_ptr<Collider2D> const& collider);
    void clear_overlapped_this_frame();

    void update_center_and_corners();

    glm::vec2 offset = {};

    bool is_trigger = false;
    bool is_static = false;

    ColliderType2D collider_type = ColliderType2D::Circle;

    float width = 1.0f; // For rectangle
    float height = 1.0f; // For rectangle

    float radius = 1.0f; // For circle

    // FIXME: This should belong to some kind of Rigidbody component.
    float drag = 0.01f;
    glm::vec2 velocity = {};

private:
    void compute_axes(glm::vec2 const& center, glm::quat const& rotation);

    std::array<glm::vec2, 4> m_corners = {}; // For rectangle, calculated each frame
    std::array<glm::vec2, 2> m_axes = {}; // For rectangle, calculated each frame

    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_inside_trigger = {};
    std::vector<std::weak_ptr<Collider2D>> m_inside_trigger_vector = {};

    std::vector<std::weak_ptr<Collider2D>> m_overlapped_this_frame = {};
    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_overlapped_this_frame_map = {};

    std::shared_ptr<Entity> m_debug_drawing_entity = nullptr;
    std::shared_ptr<DebugDrawing> m_debug_drawing = nullptr;
};
