#include <imgui.h>

#include "Entity.h"
#include "Curve.h"

#include "imgui_extensions.h"

#include <glm/gtc/type_ptr.inl>
#include <iostream>
#include <implot.h>
#include "Game/LevelController.h"

std::shared_ptr<Curve> Curve::create()
{
    return std::make_shared<Curve>(AK::Badge<Curve> {});
}

Curve::Curve(AK::Badge<Curve>)
{
}

Curve::Curve() = default;

void Curve::draw_editor()
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
        if (points.size() == 0)
        {
            points.push_back({ 0.1f, 0.5f });
        }
        else
        {
            points.push_back({ points.back().x + 0.1f, points.back().y });

            points.back().x = glm::clamp(points.back().x, 0.0f, 1.0f);
        }
    }

    if (ImPlot::BeginPlot("Path visualised")) 
    {
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
        ImPlot::SetupLegend(ImPlotFlags_NoLegend);

        std::vector<float> xs, ys;
        for (const auto& p : points) 
        {
            xs.push_back(p.x);
            ys.push_back(p.y);
        }

        ImPlot::PlotLine("##Line", xs.data(), ys.data(), points.size());
        
        for (u32 i = 0; i < points.size(); i++)
        {
            double px = xs[i];
            double py = ys[i];
            if (ImPlot::DragPoint(i, &px, &py, ImVec4(0, 0.9f, 0, 1), 4))
            {
                points[i].x = px;

                float left_clamp = 0.0f;
                float right_clamp = 1.0f;

                if (i > 0)
                {
                    left_clamp = points[i - 1].x;
                }

                if (i < points.size() - 1)
                {
                    right_clamp = points[i + 1].x;
                }

                points[i].x = glm::clamp(points[i].x, left_clamp, right_clamp);

                points[i].y = py;
            }
        }

        ImPlot::EndPlot();
    }
}

float Curve::length() const
{
    float distance = 0.0f;
    for (u32 i = 0; i < points.size() - 1; i++)
    {
        distance += glm::distance(points[i], points[i + 1]);
    }

    return distance;
}

glm::vec2 Curve::get_point_at(float x) const
{
    if (points.empty()) 
    {
        return glm::vec2(0.0f, 0.0f);
    }

    if (points.size() == 1)
    {
        return points[0];
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


    return glm::vec2(0.0f, 0.0f);
}

float Curve::get_y_at(float x) const
{
    if (points.empty()) 
    {
        return 0.0f;
    }

    x = glm::clamp(x, 0.0f, 1.0f);

    for (u32 i = 0; i < points.size() - 1; i++) 
    {
        if (x >= points[i].x && x <= points[i + 1].x) 
        {
            float x0 = points[i].x;
            float y0 = points[i].y;
            float x1 = points[i + 1].x;
            float y1 = points[i + 1].y;

            float t = (x - x0) / (x1 - x0);
            float y = y0 + t * (y1 - y0);
            return y;
        }
    }

    return 0.0f;
}

void Curve::add_points(std::initializer_list<glm::vec2> new_points)
{
    points.insert(points.end(), new_points.begin(), new_points.end());
}