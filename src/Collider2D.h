#pragma once
#include "Component.h"
#include "AK/Types.h"
#include "AK/Badge.h"
#include "glm/glm.hpp"

#include <array>

enum class ColliderType2D
{
    Rectangle,
    Circle
};

class Collider2D final : public Component
{
public:
    static std::shared_ptr<Collider2D> create();
    static std::shared_ptr<Collider2D> create(ColliderType2D const collider_type, float const radius, bool const is_static = false);
    static std::shared_ptr<Collider2D> create(ColliderType2D const collider_type, glm::vec2 const bounds_dimensions,
                                              bool const is_static = false);
    static std::shared_ptr<Collider2D> create(float const width, float const height, bool const is_static = false);

    // CircleCollision
    explicit Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, float const radius, bool const is_static);

    // RectangleCollision
    explicit Collider2D(AK::Badge<Collider2D>, ColliderType2D const collider_type, glm::vec2 const bounds_dimensions, bool const is_static);
    explicit Collider2D(AK::Badge<Collider2D>, float const width, float const height, bool const is_static);

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void start() override;
    virtual void update() override;

    // CIRCLE X CIRCLE
    void separate(glm::vec3 const& center1, glm::vec3 const& center2, float const radius1, float const radius2) const;

    // CIRCLE X RECTANGLE
    void separate(glm::vec2 const center, float const radius, glm::vec2 const nearest_point, float const left, float const right,
                  float const top, float const bottom, bool const negate) const;

    // RECTANGLE X RECTANGLE
    void separate(float const left1, float const left2, float const right1, float const right2, float const top1, float const top2,
                  float const bottom1, float const bottom2, bool const negate) const;

    ColliderType2D get_collider_type() const;
    float get_radius_2d() const;
    glm::vec2 get_center_2d() const;
    glm::vec2 get_bounds_dimensions_2d() const;

    bool is_static() const;
    void set_static(bool const value);

    bool overlaps(Collider2D const& other) const;

    bool overlaps_one_way(Collider2D const& other) const;
    void compute_axes(glm::vec2 const& center, float const angle);

private:
    // This is distance from center_2d of a rectangle to the bounds.
    // For example, if we have a 1x1 square collider, the center is in the middle
    // and bounds_dimensions = {0.5, 0.5}.
    glm::vec2 m_bounds_dimensions = {};
    float m_radius = 0.0f;

    bool m_is_static = false;

    ColliderType2D m_collider_type = ColliderType2D::Circle;

    // OBB:

    std::array<glm::vec2, 4> m_corners = {};
    std::array<glm::vec2, 2> m_axes = {};

    float m_width = 1.0f;
    float m_height = 1.0f;

    std::array<float, 2> m_origins = {};
};
