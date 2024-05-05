#include <imgui.h>

#include "Entity.h"
#include "Path.h"

#include "imgui_extensions.h"

#include <glm/gtc/type_ptr.inl>
#include <iostream>

std::shared_ptr<Path> Path::create()
{
    return std::make_shared<Path>(AK::Badge<Path> {});
}

Path::Path(AK::Badge<Path>)
{
}

void Path::draw_editor()
{
    for (u32 i = 0; i < points.size(); i++)
    {
        ImGuiEx::InputFloat2(("Position##" + std::to_string(i)).c_str(), glm::value_ptr(points[i]));
        ImGui::SameLine();
        if (ImGui::Button(("Remove point##" + std::to_string(i)).c_str()))
        {
            points.erase(points.begin() + i);
        }
    }

    if (ImGui::Button("Add point"))
    {
        points.emplace_back(glm::vec2());
    }
    ImGui::SameLine();
    if (ImGui::Button("Add point from position"))
    {
        points.push_back(glm::vec2(entity->transform->get_local_position().x, entity->transform->get_local_position().z));
    }
}
