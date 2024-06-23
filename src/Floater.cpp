#include "Floater.h"

#include "AK/AK.h"
#include "Entity.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#if EDITOR
#include <imgui.h>
#include <imgui_extensions.h>
#endif

Floater::Floater(AK::Badge<Floater>)
{
}

std::shared_ptr<Floater> Floater::create()
{
    return std::make_shared<Floater>(AK::Badge<Floater> {});
}

std::shared_ptr<Floater> Floater::create(std::weak_ptr<Water> const& water, float const sink, float const side_floaters_offset,
                                         float const side_rotation_strength, float const forward_rotation_strength,
                                         float const forward_floaters_offset)
{
    auto floater = std::make_shared<Floater>(AK::Badge<Floater> {});
    floater->water = water;
    floater->sink = sink;
    floater->side_floaters_offset = side_floaters_offset;
    floater->side_roation_strength = side_rotation_strength;
    floater->forward_rotation_strength = forward_rotation_strength;
    floater->forward_floaters_offest = forward_floaters_offset;
    return floater;
}

#if EDITOR
void Floater::draw_editor()
{
    Component::draw_editor();

    ImGui::InputFloat("Sink", &sink);
    ImGui::Text("Forward floaters");
    ImGui::InputFloat("Forward rotation strength", &forward_rotation_strength);
    ImGui::InputFloat("Forward floaters offset", &forward_floaters_offest);
    ImGui::Text("Side floaters");
    ImGui::InputFloat("Side rotation strength", &side_roation_strength);
    ImGui::InputFloat("Side floaters offset", &side_floaters_offset);
    ImGuiEx::draw_ptr("Water", water);
}
#endif

void Floater::awake()
{
    set_can_tick(true);
}

void Floater::update()
{
    if (water.expired())
        return;

    auto water_ptr = water.lock();

    glm::vec3 const position = entity->transform->get_position();
    glm::vec2 const position_2d = AK::convert_3d_to_2d(position);
    glm::vec2 const movement_direction = AK::convert_3d_to_2d(glm::normalize(entity->transform->get_forward()));
    glm::vec2 perpendicular_to_movement_direction = {movement_direction.y, -movement_direction.x};
    float const height_to_the_left = water_ptr->get_wave_height(position_2d + perpendicular_to_movement_direction * side_floaters_offset);
    float const height_to_the_right = water_ptr->get_wave_height(position_2d + perpendicular_to_movement_direction * -side_floaters_offset);
    float const height = water_ptr->get_wave_height(position_2d) - sink;
    float const height_at_front = water_ptr->get_wave_height(position_2d + movement_direction * forward_floaters_offest);
    float const height_at_back = water_ptr->get_wave_height(position_2d + movement_direction * -forward_floaters_offest);

    entity->transform->set_position(glm::vec3(position_2d.x, height, position_2d.y));

    float rotation_value = (height_at_front - height_at_back) * forward_rotation_strength;
    glm::quat const rotation = glm::angleAxis(rotation_value, entity->transform->get_right());
    glm::quat const rotation_forward_axis =
        glm::angleAxis((height_to_the_left - height_to_the_right) * side_roation_strength, entity->transform->get_forward());
    glm::quat const final_rotation = rotation_forward_axis * rotation;
    glm::vec3 const euler = glm::degrees(glm::eulerAngles(final_rotation));
    glm::vec3 const current_rotation = entity->transform->get_euler_angles();

    entity->transform->set_euler_angles(glm::vec3(euler.x, current_rotation.y, euler.z));
}
