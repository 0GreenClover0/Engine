#include "FloatersManager.h"

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

FloatersManager::FloatersManager(AK::Badge<FloatersManager>)
{
}

std::shared_ptr<FloatersManager> FloatersManager::create()
{
    return std::make_shared<FloatersManager>(AK::Badge<FloatersManager> {});
}

#if EDITOR
void FloatersManager::draw_editor()
{
    Component::draw_editor();
    ImGuiEx::draw_ptr("Water", water);

    ImGui::Text("Big Boat Settings");
    ImGui::PushID("Big Boat Settings");
    ImGui::InputFloat("Sink rate", &big_boat_settings.sink_rate);
    ImGui::InputFloat("Side rotation strength", &big_boat_settings.side_rotation_strength);
    ImGui::InputFloat("Forward rotation strength", &big_boat_settings.forward_rotation_strength);
    ImGui::InputFloat("Side floater offset", &big_boat_settings.side_floaters_offset);
    ImGui::PopID();

    ImGui::Text("Small Boat Settings");
    ImGui::PushID("Small Boat Settings");
    ImGui::InputFloat("Sink rate", &small_boat_settings.sink_rate);
    ImGui::InputFloat("Side rotation strength", &small_boat_settings.side_rotation_strength);
    ImGui::InputFloat("Forward rotation strength", &small_boat_settings.forward_rotation_strength);
    ImGui::InputFloat("Side floater offset", &small_boat_settings.side_floaters_offset);
    ImGui::PopID();

    ImGui::Text("Medium Boat Settings");
    ImGui::PushID("Medium Boat Settings");
    ImGui::InputFloat("Sink rate", &medium_boat_settings.sink_rate);
    ImGui::InputFloat("Side rotation strength", &medium_boat_settings.side_rotation_strength);
    ImGui::InputFloat("Forward rotation strength", &medium_boat_settings.forward_rotation_strength);
    ImGui::InputFloat("Side floater offset", &medium_boat_settings.side_floaters_offset);
    ImGui::PopID();

    ImGui::Text("Tool Boat Settings");
    ImGui::PushID("Tool Boat Settings");
    ImGui::InputFloat("Sink rate", &tool_boat_settings.sink_rate);
    ImGui::InputFloat("Side rotation strength", &tool_boat_settings.side_rotation_strength);
    ImGui::InputFloat("Forward rotation strength", &tool_boat_settings.forward_rotation_strength);
    ImGui::InputFloat("Side floater offset", &tool_boat_settings.side_floaters_offset);
    ImGui::PopID();

    ImGui::Text("Pirate Boat Settings");
    ImGui::PushID("Pirate Boat Settings");
    ImGui::InputFloat("Sink rate", &pirate_boat_settings.sink_rate);
    ImGui::InputFloat("Side rotation strength", &pirate_boat_settings.side_rotation_strength);
    ImGui::InputFloat("Forward rotation strength", &pirate_boat_settings.forward_rotation_strength);
    ImGui::InputFloat("Side floater offset", &pirate_boat_settings.side_floaters_offset);
    ImGui::PopID();
}
#endif
