#include "DebugDrawing.h"

#include "Cube.h"
#include "Engine.h"
#include "Entity.h"
#include "Globals.h"
#include "ResourceManager.h"
#include "Sphere.h"
#include "AK/AK.h"

DebugDrawing::DebugDrawing(AK::Badge<DebugDrawing>)
{
}

std::shared_ptr<DebugDrawing> DebugDrawing::create()
{
    return std::make_shared<DebugDrawing>(AK::Badge<DebugDrawing> {});
}

DebugDrawing::DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, float const radius, double const time) :
    m_lifetime(time), m_radius(radius), m_position(position)
{
}

std::shared_ptr<DebugDrawing> DebugDrawing::create(glm::vec3 position, float radius, double time)
{
    return std::make_shared<DebugDrawing>(AK::Badge<DebugDrawing> {}, position, radius, time);
}

DebugDrawing::DebugDrawing(AK::Badge<DebugDrawing>, glm::vec3 const position, glm::vec3 const euler_angles, glm::vec3 const extents,
    double const time) : m_type(DrawingType::Box), m_lifetime(time), m_position(position),
    m_euler_angles(euler_angles), m_extents(extents)
{
}

std::shared_ptr<DebugDrawing> DebugDrawing::create(glm::vec3 position, glm::vec3 euler_angles, glm::vec3 extents,
    double time)
{
    return std::make_shared<DebugDrawing>(AK::Badge<DebugDrawing> {}, position, euler_angles, extents, time);
}

void DebugDrawing::initialize()
{
    Component::initialize();

    set_can_tick(true);
    
    auto const light_source_shader = ResourceManager::get_instance().load_shader("./res/shaders/light_source.hlsl", "./res/shaders/light_source.hlsl");
    auto const plain = Material::create(light_source_shader);
    std::shared_ptr<Sphere> sphere = nullptr;
    std::shared_ptr<Cube> box = nullptr;

    switch (m_type)
    {
    case DrawingType::Sphere:
        sphere = entity->add_component<Sphere>(Sphere::create(m_radius, 10, 10,"./res/textures/white.jpg", plain));
        entity->transform->set_local_position(m_position);
        sphere->set_rasterizer_draw_type(RasterizerDrawType::Wireframe);
        break;

    case DrawingType::Box:
        box = entity->add_component<Cube>(Cube::create(plain));
        entity->transform->set_local_position(m_position);
        entity->transform->set_euler_angles(m_euler_angles);
        entity->transform->set_local_scale(m_extents);
        box->set_rasterizer_draw_type(RasterizerDrawType::Wireframe);
        break;

    default:
        std::unreachable();
    }
}

void DebugDrawing::update()
{
    if (glm::abs(m_lifetime) < 0.00001) // Epsilon
        return;

    m_current_time += delta_time;

    if (m_current_time > m_lifetime)
        entity->destroy_immediate();
}

void DebugDrawing::draw_editor()
{
    Component::draw_editor();

    if (glm::abs(m_lifetime) < 0.000001)
        ImGui::Text("Time set to 0 will render the drawing infinitely.");

    ImGui::InputDouble("Time", &m_lifetime);
}
