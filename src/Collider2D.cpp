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
    : is_static(is_static), collider_type(ColliderType2D::Circle), radius(radius)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, glm::vec2 const bounds_dimensions, bool const is_static)
    : is_static(is_static), collider_type(ColliderType2D::Rectangle), width(bounds_dimensions.x), height(bounds_dimensions.y)
{
}

Collider2D::Collider2D(AK::Badge<Collider2D>, float const width, float const height, bool const is_static)
    : is_static(is_static), collider_type(ColliderType2D::Rectangle), width(width), height(height)
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

    if (collider_type == ColliderType2D::Circle)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Use RADIUS to control collider size.");
        m_debug_drawing->set_drawing_type(DrawingType::Sphere);
    }
    else if (collider_type == ColliderType2D::Rectangle)
    {
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Use EXTENTS to control collider size.");
        m_debug_drawing->set_drawing_type(DrawingType::Box);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Dropdown list
    std::array const collider_types = {"Rectangle", "Circle"};
    i32 current_item_index = static_cast<i32>(collider_type);
    if (ImGui::Combo("Collider Type", &current_item_index, collider_types.data(), collider_types.size()))
    {
        is_dirty = true;

        collider_type = static_cast<ColliderType2D>(current_item_index);
    }

    if (collider_type == ColliderType2D::Circle)
    {
        float const previous_radius = radius;
        ImGui::InputFloat("Radius", &radius);

        if (!AK::Math::are_nearly_equal(previous_radius, radius))
        {
            is_dirty = true;
            set_radius_2d(radius);
            m_debug_drawing->set_radius(radius);
        }
    }
    else if (collider_type == ColliderType2D::Rectangle)
    {
        std::array extents = {width, height};
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

    ImGui::Checkbox("Trigger", &is_trigger);

    ImGui::Checkbox("Static", &is_static);

    ImGui::Spacing();
    ImGui::Spacing();

    if (is_dirty)
    {
        Debug::log("changed");
        if (collider_type == ColliderType2D::Circle)
        {
            m_debug_drawing->set_drawing_type(DrawingType::Sphere);
        }
        else if (collider_type == ColliderType2D::Rectangle)
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

    switch (collider_type)
    {
    case ColliderType2D::Circle:
        m_debug_drawing_entity = Debug::draw_debug_sphere({0.0f, 0.0f, 0.0f}, radius);
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    case ColliderType2D::Rectangle:
        m_debug_drawing_entity = Debug::draw_debug_box({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {width * 2.0f, 0.5f, height * 2.0f});
        m_debug_drawing_entity->transform->set_parent(entity->transform);
        break;

    default:
        std::unreachable();
    }

    m_debug_drawing = m_debug_drawing_entity->get_component<DebugDrawing>();
    m_debug_drawing->set_radius(radius);
    m_debug_drawing->set_extents({width * 2.0f, 0.25f, height * 2.0f});
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

void Collider2D::set_collider_type(ColliderType2D new_collider_type)
{
    collider_type = new_collider_type;

    if (new_collider_type == ColliderType2D::Circle)
    {
        m_debug_drawing->set_drawing_type(DrawingType::Sphere);
    }
    else if (new_collider_type == ColliderType2D::Rectangle)
    {
        m_debug_drawing->set_drawing_type(DrawingType::Box);
    }
}

void Collider2D::set_radius_2d(float const new_radius)
{
    radius = new_radius;
}

float Collider2D::get_radius_2d() const
{
    return radius;
}

void Collider2D::set_extents(glm::vec2 const extents)
{
    width = extents.x;
    height = extents.y;
}

glm::vec2 Collider2D::get_extents() const
{
    return {width, height};
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

glm::vec2 Collider2D::get_bounds_dimensions_2d() const
{
    return {width, height};
}

void Collider2D::set_bounds_dimensions_2d(float const new_width, float const new_height)
{
    width = new_width;
    height = new_height;
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

void Collider2D::physics_update()
{
    update_center_and_corners();

    if (glm::epsilonEqual(velocity, {0.0f, 0.0f}, 0.001f) != glm::bvec2(true, true))
    {
        entity->transform->set_position(entity->transform->get_position()
                                        + AK::convert_2d_to_3d(velocity) * static_cast<float>(delta_time));

        velocity = AK::move_towards(velocity, {0.0f, 0.0f}, drag);
    }
}

void Collider2D::add_force(glm::vec2 const force)
{
    velocity += force;
}

void Collider2D::update_center_and_corners()
{
    glm::vec2 const position = get_center_2d();

    // HACK: For some reason we need to negate the angle here. Not sure why, but this fixes most
    //       of our collision problems.
    float const angle = -glm::eulerAngles(entity->transform->get_rotation()).y;
    compute_axes(position, angle);
    m_debug_drawing_entity->transform->set_position(AK::convert_2d_to_3d(position));
}

// NOTE: Should be called everytime the position has changed.
//       Currently we just update it every frame.
void Collider2D::compute_axes(glm::vec2 const& center, float const angle)
{
    // Create a 2D rotation matrix
    glm::mat2 const rotation_matrix = glm::mat2(glm::vec2(cos(angle), sin(angle)), glm::vec2(-sin(angle), cos(angle)));

    // Define the half-dimensions of the collider
    glm::vec2 const half_extents = glm::vec2(width, height) * 0.5f;

    // Calculate the rotated corners
    std::array const corners_local = {
        glm::vec2(-half_extents.x, -half_extents.y),
        glm::vec2(half_extents.x, -half_extents.y),
        glm::vec2(half_extents.x, half_extents.y),
        glm::vec2(-half_extents.x, half_extents.y),
    };

    for (u32 i = 0; i < 4; ++i)
    {
        m_corners[i] = center + rotation_matrix * corners_local[i];
    }

    // Compute the axes for the SAT (Separating Axis Theorem) test
    m_axes[0] = glm::normalize(m_corners[1] - m_corners[0]);
    m_axes[1] = glm::normalize(m_corners[3] - m_corners[0]);

    m_debug_drawing->set_radius(radius);
    m_debug_drawing->set_extents({width, 0.25f, height});
}
