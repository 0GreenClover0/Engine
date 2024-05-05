#include <imgui.h>

#include "Entity.h"
#include "Path.h"
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
        ImGui::InputFloat2(("Position##" + std::to_string(i)).c_str(), glm::value_ptr(points[i]));
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
}

float Path::length() const
{
    float distance = 0.0f;
    for (u32 i = 0; i < points.size() - 1; i++)
    {
        distance += glm::distance(points[i], points[i + 1]);
    }

    return distance;
}

glm::vec2 Path::get_point_at(float x) const
{
    if (points.empty() || points.size() == 1)
    {
        return {0.0f, 0.0f};
    }

    if (x < 0.0f || x > 1.0f)
    {
        Debug::log("Supplied x is out of range <0, 1>, clamped to nearest value", DebugType::Warning);
    }

    x = glm::clamp(x, 0.0f, 1.0f);
    
    float const path_length = length();
    float const desire_length = path_length * x;
    float distance = 0.0f;

    for (u32 i = 0; i < points.size() - 1; i++)
    {
        float const segment_length = glm::distance(points[i], points[i + 1]);
        if (distance + segment_length < desire_length)
        {
            distance += segment_length;
        }
        else
        {
            return (((desire_length - distance) / (segment_length)) * (points[i + 1] - points[i])) + points[i];
        }
    }

    return {0.0f, 0.0f};
}
