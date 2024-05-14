#pragma once

#include "Cube.h"
#include "Drawable.h"
#include "ResourceManager.h"
#include "Sphere.h"

enum class DrawingType
{
    Sphere = 0,
    Box = 1,
};

NON_SERIALIZED
class DebugDrawing final : public Component
{
public:
    // Default (simple sphere)
    explicit DebugDrawing(AK::Badge<DebugDrawing>);
    static std::shared_ptr<DebugDrawing> create();

    // Sphere
    explicit DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, float const radius = 1.0f, double const time = 0.0);
    static std::shared_ptr<DebugDrawing> create(glm::vec3 const position, float const radius = 1.0f, double const time = 0.0);

    // Box
    explicit DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, glm::vec3 const euler_angles = { 0.0f, 0.0f, 0.0f },
                          glm::vec3 const extents = { 0.25f, 0.25f, 0.25f }, double const time = 0.0);
    static std::shared_ptr<DebugDrawing> create(glm::vec3 const position, glm::vec3 const euler_angles = { 0.0f, 0.0f, 0.0f },
                                                glm::vec3 const extents = { 0.25f, 0.25f, 0.25f }, double const time = 0.0);

    virtual void initialize() override;
    virtual void update() override;

    // TODO: This should be in an update_editor() method
    virtual void draw_editor() override;

    virtual void reprepare() override;

    float get_radius() const;
    void set_radius(float const radius);

    glm::vec3 get_extents() const;
    void set_extents(glm::vec3 const& extents);

private:
    void create_box(bool const is_reload);
    void create_sphere(bool const is_reload);

    DrawingType m_type = DrawingType::Sphere;
    DrawingType m_previous_drawing_type = DrawingType::Sphere;

    // TODO: Make this work also in editor, currently time is only counted in play mode.
    double m_lifetime = 0.0;
    float m_radius = 1.0f;
    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_euler_angles = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_extents = { m_radius, m_radius, m_radius };

    double m_current_time = 0.0;

    std::shared_ptr<Shader> m_light_source_shader = nullptr;
    std::shared_ptr<Material> m_plain_material = nullptr;
    std::shared_ptr<Sphere> m_sphere_component = nullptr;
    std::shared_ptr<Cube> m_box_component = nullptr;
};
