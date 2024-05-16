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

    virtual void draw_editor() override;

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;

    void apply_mtv(glm::vec2 const mtv) const;

    ColliderType2D get_collider_type() const;

    bool is_trigger() const;
    void set_is_trigger(bool const is_trigger);

    bool is_static() const;
    void set_is_static(bool const value);

    void set_radius_2d(float const new_radius);
    float get_radius_2d() const;

    void set_extents(glm::vec2 const extents);
    glm::vec2 get_extents() const;

    glm::vec2 get_center_2d() const;
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

    glm::vec2 offset = {};

private:
    void update_center_and_corners();
    void compute_axes(glm::vec2 const& center, float const angle);

    bool m_is_trigger = false;
    bool m_is_static = false;

    ColliderType2D m_collider_type = ColliderType2D::Circle;
    ColliderType2D m_previous_collider_type = ColliderType2D::Circle;

    std::array<glm::vec2, 4> m_corners = {}; // For rectangle, calculated each frame
    std::array<glm::vec2, 2> m_axes = {}; // For rectangle, calculated each frame

    float m_width = 1.0f; // For rectangle
    float m_height = 1.0f; // For rectangle

    float m_radius = 1.0f; // For circle

    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_inside_trigger = {};
    std::vector<std::weak_ptr<Collider2D>> m_inside_trigger_vector = {};

    std::vector<std::weak_ptr<Collider2D>> m_overlapped_this_frame = {};
    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_overlapped_this_frame_map = {};

    std::shared_ptr<Entity> m_debug_drawing_entity = nullptr;
    std::shared_ptr<DebugDrawing> m_debug_drawing = nullptr;
};
