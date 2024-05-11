#pragma once

#include "Drawable.h"

enum class DrawingType
{
    Sphere,
    Box
};

NON_SERIALIZED
class DebugDrawing final : public Component
{
public:
    // Default (simple sphere)
    explicit DebugDrawing(AK::Badge<DebugDrawing>);
    static std::shared_ptr<DebugDrawing> create();

    // Sphere
    explicit DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, float const radius = 0.1f, double const time = 0.0);
    static std::shared_ptr<DebugDrawing> create(glm::vec3 const position, float const radius = 0.1f, double const time = 0.0);

    // Box
    explicit DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, glm::vec3 const euler_angles = { 0.0f, 0.0f, 0.0f },
                          glm::vec3 const extents = { 0.25f, 0.25f, 0.25f }, double const time = 0.0);
    static std::shared_ptr<DebugDrawing> create(glm::vec3 const position, glm::vec3 const euler_angles = { 0.0f, 0.0f, 0.0f },
                                                glm::vec3 const extents = { 0.25f, 0.25f, 0.25f }, double const time = 0.0);

    virtual void initialize() override;
    virtual void update() override;
    virtual void draw_editor() override;

private:
    DrawingType m_type = DrawingType::Sphere;

    // TODO: Make this work also in editor, currently time is only counted in play mode.
    double m_lifetime = 0.0;
    float m_radius = 0.1f;
    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_euler_angles = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_extents = { 0.25f, 0.25f, 0.25f };

    double m_current_time = 0.0;
};
