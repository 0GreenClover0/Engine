#include "Path.h"

#include "Entity.h"
#include "Game/LevelController.h"

#include <glm/gtc/type_ptr.inl>

#include <iostream>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#include <implot.h>
#endif

std::shared_ptr<Path> Path::create()
{
    return std::make_shared<Path>(AK::Badge<Path> {});
}

Path::Path(AK::Badge<Path>)
{
}

#if EDITOR
void Path::draw_editor()
{
    Component::draw_editor();

    if (ImPlot::BeginPlot("Path visualised") or true)
    {
        ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 2.0f);
        ImPlot::SetupLegend(ImPlotFlags_NoLegend);

        if (LevelController::get_instance() != nullptr)
        {
            ImPlot::SetupAxesLimits(-LevelController::get_instance()->playfield_width, LevelController::get_instance()->playfield_width,
                                    -LevelController::get_instance()->playfield_height, LevelController::get_instance()->playfield_height,
                                    ImGuiCond_Once);
        }

        ImGui::Checkbox("Reverse y-axis", &m_reverse_y);

        std::vector<float> xs, ys;
        for (auto const& p : points)
        {
            xs.push_back(p.x);
            m_reverse_y ? ys.push_back(-p.y) : ys.push_back(p.y);
        }

        ImPlot::PlotLine("##Line", xs.data(), ys.data(), points.size());

        for (u32 i = 0; i < points.size(); i++)
        {
            double px = xs[i];
            double py = ys[i];
            if (ImPlot::DragPoint(i, &px, &py, ImVec4(0, 0.9f, 0, 1), 4))
            {
                points[i].x = px;
                points[i].y = m_reverse_y ? -py : py;
            }
        }

        ImPlot::EndPlot();
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

    for (u32 i = 0; i < points.size(); i++)
    {
        ImGuiEx::InputFloat2(("Position##" + std::to_string(i)).c_str(), glm::value_ptr(points[i]));
        ImGui::SameLine();
        if (ImGui::Button(("Remove point##" + std::to_string(i)).c_str()))
        {
            points.erase(points.begin() + i);
        }
    }

    ImGui::Separator();

    static glm::vec2 shift = {0.0f, 0.0f};

    ImGui::InputFloat2("Shift", &shift[0]);

    if (ImGui::Button("Applay shift"))
    {
        shift_all_by(shift);
    }
}
#endif

void Path::shift_all_by(glm::vec2 value)
{
    for (auto& point : points)
    {
        point += value;
    }
}
