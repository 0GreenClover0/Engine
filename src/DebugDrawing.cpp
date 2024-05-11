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
    m_light_source_shader = ResourceManager::get_instance().load_shader("./res/shaders/light_source.hlsl", "./res/shaders/light_source.hlsl");
    m_plain_material = Material::create(m_light_source_shader);

    switch (m_type)
    {
    case DrawingType::Sphere:
        create_sphere(false);
        break;

    case DrawingType::Box:
        create_box(false);
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

    std::array const drawing_types = {"Sphere", "Box"};
    i32 current_item_index = static_cast<i32>(m_type);
    if (ImGui::Combo("Debug Drawing Type", &current_item_index, drawing_types.data(), drawing_types.size()))
    {
        m_previous_drawing_type = m_type;
        m_type = static_cast<DrawingType>(current_item_index);
        reprepare();
    }

    if (glm::abs(m_lifetime) < 0.000001)
        ImGui::Text("Time set to 0 will render the drawing infinitely.");

    ImGui::InputDouble("Lifetime (in game)", &m_lifetime);

    if (m_type == DrawingType::Sphere)
    {
        ImGui::InputFloat("Radius", &m_radius);
        entity->transform->set_local_scale(glm::vec3(m_radius * 10.0f));
    }
    else if (m_type == DrawingType::Box)
    {
        std::array extents = {m_extents.x, m_extents.y, m_extents.z};
        ImGui::InputFloat3("Extents", extents.data());
        m_extents = { extents[0], extents[1], extents[2] };
        entity->transform->set_local_scale(glm::vec3(extents[0], extents[1], extents[2]));
    }
}

void DebugDrawing::reprepare()
{
    Component::reprepare();

    switch (m_previous_drawing_type)
    {
    case DrawingType::Sphere:
        m_sphere_component->destroy_immediate();
        m_sphere_component = nullptr;
        break;

    case DrawingType::Box:
        m_box_component->destroy_immediate();
        m_box_component = nullptr;
        break;

    default:
        break;
    }

    switch (m_type)
    {
    case DrawingType::Sphere:
        create_sphere(true);
        break;

    case DrawingType::Box:
        create_box(true);
        break;

    default:
        break;
    }
}

void DebugDrawing::create_box(bool const is_reload)
{
    m_box_component = entity->add_component<Cube>(Cube::create(m_plain_material));

    if (!is_reload)
    {
        entity->transform->set_local_position(m_position);
        entity->transform->set_euler_angles(m_euler_angles);
        entity->transform->set_local_scale(m_extents);
    }

    m_box_component->set_rasterizer_draw_type(RasterizerDrawType::Wireframe);
}

void DebugDrawing::create_sphere(bool const is_reload)
{
    m_sphere_component = entity->add_component<Sphere>(Sphere::create(m_radius * 10.0f, 10, 10, "./res/textures/white.jpg", m_plain_material));

    if (!is_reload)
    {
        entity->transform->set_local_position(m_position);
    }

    m_sphere_component->set_rasterizer_draw_type(RasterizerDrawType::Wireframe);
}
