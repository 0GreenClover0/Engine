#include "Collider2D.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "DebugDrawing.h"
#include "Engine.h"
#include "Entity.h"
#include "Globals.h"
#include "PhysicsEngine.h"
#include "imgui_extensions.h"

#include <glm/gtc/type_ptr.inl>

std::shared_ptr<Collider2D> Collider2D::create()
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, 1.0f, false);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(float const radius, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, radius, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(glm::vec2 const bounds_dimensions, bool is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, bounds_dimensions, is_static);
    return collider_2d;
}

std::shared_ptr<Collider2D> Collider2D::create(float const width, float const height, bool const is_static)
{
    auto collider_2d = std::make_shared<Collider2D>(AK::Badge<Collider2D> {}, width, height, is_static);
    return collider_2d;
}

bool Collider2D::is_point_inside_obb(glm::vec2 const& point, std::array<glm::vec2, 4> const& rectangle_corners) const
{
    glm::vec2 const ap = point - rectangle_corners[0]; // Vector from one rectangle corner to point
    glm::vec2 const ab = rectangle_corners[1] - rectangle_corners[0]; // One rectangle axis
    glm::vec2 const ad = rectangle_corners[3] - rectangle_corners[0]; // Another rectangle axis

    // Dot products
    float const ap_dot_ab = glm::dot(ap, ab);
    float const ab_dot_ab = glm::dot(ab, ab);
    float const ap_dot_ad = glm::dot(ap, ad);
    float const ad_dot_ad = glm::dot(ad, ad);

    return (0.0f <= ap_dot_ab && ap_dot_ab <= ab_dot_ab) && (0.0f <= ap_dot_ad && ap_dot_ad <= ad_dot_ad);
}

Collider2D::Collider2D(AK::Badge<Collider2D>, float const radius, bool const is_static)
    : m_is_static(is_static), m_collider_type(ColliderType2D::Circle), m_radius(radius)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, glm::vec2 const bounds_dimensions, bool const is_static)
    : m_is_static(is_static), m_collider_type(ColliderType2D::Rectangle), m_width(bounds_dimensions.x), m_height(bounds_dimensions.y)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, float const width, float const height, bool const is_static)
    : m_is_static(is_static), m_collider_type(ColliderType2D::Rectangle), m_width(width), m_height(height)
{
}

void Collider2D::draw_editor()
{
    Component::draw_editor();

    bool is_dirty = false;

    glm::vec2 const previous_offset = offset;
    ImGuiEx::InputFloat2("Offset", glm::value_ptr(offset));

    if (glm::epsilonEqual(previous_offset, offset, {0.0001f, 0.0001f}) != glm::bvec2(true, true))
    {
        is_dirty = true;
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (m_collider_type == ColliderType2D::Circle)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Use RADIUS to control collider size.");
        m_debug_drawing->set_drawing_type(DrawingType::Sphere);
    }
    else if (m_collider_type == ColliderType2D::Rectangle)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Use EXTENTS to control collider size.");
        m_debug_drawing->set_drawing_type(DrawingType::Box);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Dropdown list
    std::array const collider_types = {"Rectangle", "Circle"};
    i32 current_item_index = static_cast<i32>(m_collider_type);
    if (ImGui::Combo("Collider Type", &current_item_index, collider_types.data(), collider_types.size()))
    {
        m_previous_collider_type = m_collider_type;

        is_dirty = true;

        m_collider_type = static_cast<ColliderType2D>(current_item_index);
    }

    if (m_collider_type == ColliderType2D::Circle)
    {
        float const previous_radius = m_radius;
        ImGui::InputFloat("Radius", &m_radius);

        if (!AK::Math::are_nearly_equal(previous_radius, m_radius))
        {
            is_dirty = true;
            set_radius_2d(m_radius);
            m_debug_drawing->set_radius(m_radius);
        }
    }
    else if (m_collider_type == ColliderType2D::Rectangle)
    {
        std::array extents = {m_width, m_height};
        std::array const previous_extents = extents;
        ImGui::InputFloat2("Extents", extents.data());

        if (!AK::Math::are_nearly_equal(previous_extents[0], extents[0]) || !AK::Math::are_nearly_equal(previous_extents[1], extents[1]))
        {
            is_dirty = true;
            set_extents({extents[0], extents[1]});
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Checkbox("Trigger", &m_is_trigger);

    ImGui::Checkbox("Static", &m_is_static);

    ImGui::Spacing();
    ImGui::Spacing();

    if (is_dirty)
    {
        Debug::log("changed");
        if (m_collider_type == ColliderType2D::Circle)
        {
            m_debug_drawing->set_drawing_type(DrawingType::Sphere);
        }
        else if (m_collider_type == ColliderType2D::Rectangle)
        {
            m_debug_drawing->set_drawing_type(DrawingType::Box);
        }

        update_center_and_corners();
    }
}

void Collider2D::initialize()
{
    Component::initialize();
    PhysicsEngine::get_instance()->emplace_collider(std::dynamic_pointer_cast<Collider2D>(shared_from_this()));

    switch (m_collider_type)
    {
    case ColliderType2D::Circle:
        m_debug_drawing_entity = Debug::draw_debug_sphere({0.0f, 0.0f, 0.0f}, m_radius);
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    case ColliderType2D::Rectangle:
        m_debug_drawing_entity = Debug::draw_debug_box({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {m_width * 2.0f, 0.5f, m_height * 2.0f});
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    default:
        std::unreachable();
    }

    m_debug_drawing = m_debug_drawing_entity->get_component<DebugDrawing>();
    m_debug_drawing->set_radius(m_radius);
    m_debug_drawing->set_extents({m_width * 2.0f, 0.25f, m_height * 2.0f});
    update_center_and_corners();
}

void Collider2D::uninitialize()
{
    Component::uninitialize();
    PhysicsEngine::get_instance()->remove_collider(std::dynamic_pointer_cast<Collider2D>(shared_from_this()));
}

void Collider2D::awake()
{
    set_can_tick(true);

    update_center_and_corners();
}

ColliderType2D Collider2D::get_collider_type() const
{
    return m_collider_type;
}

bool Collider2D::is_trigger() const
{
    return m_is_trigger;
}

void Collider2D::set_is_trigger(bool const is_trigger)
{
    m_is_trigger = is_trigger;
}

bool Collider2D::is_static() const
{
    return m_is_static;
}

void Collider2D::set_is_static(bool const value)
{
    if (m_is_static == value)
    {
        return;
    }

    m_is_static = value;
}

void Collider2D::set_radius_2d(float const new_radius)
{
    m_radius = new_radius;
}

float Collider2D::get_radius_2d() const
{
    return m_radius;
}

void Collider2D::set_extents(glm::vec2 const extents)
{
    m_width = extents.x;
    m_height = extents.y;
}

glm::vec2 Collider2D::get_extents() const
{
    return {m_width, m_height};
}

glm::vec2 Collider2D::get_center_2d() const
{
    // Get the entity's world position in 2D
    glm::vec2 const entity_position = AK::convert_3d_to_2d(entity->transform->get_position());

    // Get the entity's rotation quaternion
    glm::quat const entity_rotation = entity->transform->get_rotation();

    // Convert the offset to 3D for quaternion rotation
    glm::vec3 const offset_3d = AK::convert_2d_to_3d(offset);

    // Rotate the offset using the entity's rotation quaternion
    glm::vec3 const rotated_offset_3d = entity_rotation * offset_3d;

    // Convert the rotated offset back to 2D
    glm::vec2 const rotated_offset = AK::convert_3d_to_2d(rotated_offset_3d);

    // Calculate the collider's center position
    return entity_position + rotated_offset;
}

std::array<glm::vec2, 4> Collider2D::get_corners() const
{
    return m_corners;
}

std::array<glm::vec2, 2> Collider2D::get_axes() const
{
    return m_axes;
}

void Collider2D::apply_mtv(glm::vec2 const mtv) const
{
    glm::vec2 const new_position = AK::convert_3d_to_2d(entity->transform->get_position()) + mtv * 0.5f;
    entity->transform->set_position(AK::convert_2d_to_3d(new_position, entity->transform->get_position().y));
}

bool Collider2D::is_inside_trigger(std::string const& guid) const
{
    return m_inside_trigger.contains(guid);
}

std::weak_ptr<Collider2D> Collider2D::get_inside_trigger(std::string const& guid) const
{
    return m_inside_trigger.at(guid);
}

std::vector<std::weak_ptr<Collider2D>> Collider2D::get_inside_trigger_vector() const
{
    return m_inside_trigger_vector;
}

void Collider2D::add_inside_trigger(std::string const& guid, std::shared_ptr<Collider2D> const& collider)
{
    m_inside_trigger.emplace(guid, collider);
    m_inside_trigger_vector.emplace_back(collider);
}

auto Collider2D::set_inside_trigger(std::unordered_map<std::string, std::weak_ptr<Collider2D>> const& map,
                                    std::vector<std::weak_ptr<Collider2D>> const& vector) -> void
{
    m_inside_trigger = map;
    m_inside_trigger_vector = vector;
}

std::vector<std::weak_ptr<Collider2D>> Collider2D::get_all_overlapping_this_frame() const
{
    return m_overlapped_this_frame;
}

void Collider2D::add_overlapped_this_frame(std::shared_ptr<Collider2D> const& collider)
{
    m_overlapped_this_frame.emplace_back(collider);
    m_overlapped_this_frame_map.emplace(collider->guid, collider);
}

void Collider2D::clear_overlapped_this_frame()
{
    m_overlapped_this_frame.clear();
    m_overlapped_this_frame_map.clear();
}

void Collider2D::update()
{
    update_center_and_corners();
}

void Collider2D::update_center_and_corners()
{
    glm::vec2 const position = get_center_2d();
    float const angle = glm::eulerAngles(entity->transform->get_rotation()).y;
    compute_axes(position, angle);
    m_debug_drawing_entity->transform->set_position(AK::convert_2d_to_3d(position));
}

// NOTE: Should be called everytime the position has changed.
//       Currently we just update it every frame.
void Collider2D::compute_axes(glm::vec2 const& center, float const angle)
{
    // Create a 3D rotation matrix around the y-axis (y-up) and xz-plane.
    glm::mat4 const rotation_matrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));

    // Rotate the 2D vectors
    auto const x_3d = glm::vec3(rotation_matrix * glm::vec4(1, 0, 0, 1)); // Rotate the X-axis vector
    auto const z_3d = glm::vec3(rotation_matrix * glm::vec4(0, 0, 1, 1)); // Rotate the Z-axis vector

    // Convert the 3D vectors back to 2D by ignoring the y-component.
    // Not using AK, because we've been rotating individual axes.
    auto x = glm::vec2(x_3d.x, x_3d.z);
    auto y = glm::vec2(z_3d.x, z_3d.z);

    x *= m_width;
    y *= m_height;

    m_corners[0] = center - x - y;
    m_corners[1] = center + x - y;
    m_corners[2] = center + x + y;
    m_corners[3] = center - x + y;

    m_axes[0] = m_corners[1] - m_corners[0];
    m_axes[1] = m_corners[3] - m_corners[0];

    // Make the length of each axis 1/edge length so we know any
    // dot product must be less than 1 to fall within the edge.
    for (u32 a = 0; a < m_axes.size(); ++a)
    {
        m_axes[a] /= glm::pow(glm::length(m_axes[a]), 2);
    }

    m_debug_drawing->set_radius(m_radius);
    m_debug_drawing->set_extents({m_width * 2.0f, 0.25f, m_height * 2.0f});
}
