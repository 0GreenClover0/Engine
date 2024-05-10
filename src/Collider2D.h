#pragma once

#include "Component.h"
#include "AK/AK.h"
#include "AK/Types.h"
#include "AK/Badge.h"
#include "glm/glm.hpp"

#include <array>
#include <unordered_map>

enum class ColliderType2D
{
    Rectangle,
    Circle
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

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;

    // CIRCLE X CIRCLE
    void separate(Collider2D const& other, bool const is_static);

    // RECTANGLE X RECTANGLE, CIRCLE X RECTANGLE
    void separate(bool const sign, bool const is_static);

    ColliderType2D get_collider_type() const;

    bool is_trigger() const;
    void set_is_trigger(bool const is_trigger);

    bool is_static() const;
    void set_is_static(bool const value);

    float get_radius_2d() const;
    glm::vec2 get_center_2d() const;
    glm::vec2 get_bounds_dimensions_2d() const;

    bool overlaps(Collider2D& other);
    void apply_mtv(bool const sign, bool const is_static) const;

    // Internal functions meant to be used by the PhysicsEngine
    bool is_inside_trigger(std::string const& guid) const;
    std::weak_ptr<Collider2D> get_inside_trigger(std::string const& guid) const;
    std::vector<std::weak_ptr<Collider2D>> get_inside_trigger_vector() const;
    void add_inside_trigger(std::string const& guid, std::shared_ptr<Collider2D> const& collider);
    void set_inside_trigger(std::unordered_map<std::string, std::weak_ptr<Collider2D>> const& map, std::vector<std::weak_ptr<Collider2D>> const& vector);

    std::vector<std::weak_ptr<Collider2D>> get_all_overlapping_this_frame() const;
    void add_overlapped_this_frame(std::shared_ptr<Collider2D> const& collider);
    void clear_overlapped_this_frame();

private:
    glm::vec2 get_perpendicular_axis(std::array<glm::vec2, 4> const& passed_corners, u8 const index) const;
    glm::vec2 get_normal(glm::vec2 const& v) const;
    glm::vec2 project_on_axis(std::array<glm::vec2, 4> const& vertices, glm::vec2 const& axis) const;
    glm::vec2 line_intersection(glm::vec2 const& point1, glm::vec2 const& point2, glm::vec2 const& point3, glm::vec2 const& point4) const;
    void compute_axes(glm::vec2 const& center, float const angle);
    bool intersect_circle(glm::vec2 const& center, float const radius, glm::vec2 const& p1, glm::vec2 const& p2);

    bool test_collision_rectangle_rectangle(Collider2D const& obb1, Collider2D const& obb2);
    bool test_collision_circle_circle(Collider2D const& obb1, Collider2D const& obb2) const;
    bool test_collision_circle_rectangle(Collider2D& obb1, Collider2D& obb2);

    bool m_is_trigger = false;
    bool m_is_static = false;

    ColliderType2D m_collider_type = ColliderType2D::Circle;

    std::array<glm::vec2, 4> m_corners = {}; // For rectangle, calculated each frame
    std::array<glm::vec2, 2> m_axes = {};    // For rectangle, calculated each frame

    float m_width = 1.0f;  // For rectangle
    float m_height = 1.0f; // For rectangle

    float m_radius = 0.0f; // For circle
    glm::vec2 m_mtv = {};  // Minimal translation vector

    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_inside_trigger = {};
    std::vector<std::weak_ptr<Collider2D>> m_inside_trigger_vector = {};

    std::vector<std::weak_ptr<Collider2D>> m_overlapped_this_frame = {};
    std::unordered_map<std::string, std::weak_ptr<Collider2D>> m_overlapped_this_frame_map = {};
};
