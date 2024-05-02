#pragma once

#include "AK/Types.h"
#include "MainScene.h"

#include <memory>
#include <string>

#include <imgui.h>

template<class T>
void draw_ptr(std::string const& label, std::weak_ptr<T>& ptr)
{
    std::string guid;

    if (!ptr.expired())
    {
        guid = ptr.lock()->guid;
    }
    else
    {
        guid = "nullptr";
    }

    ImGui::LabelText(label.c_str(), guid.c_str());

    if (ImGui::BeginDragDropTarget())
    {
        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("guid"))
        {
            guid.resize(sizeof(i64) * 8);
            memcpy(guid.data(), payload->Data, sizeof(i64) * 8);

            if (auto const component = MainScene::get_instance()->get_component_by_guid(guid))
            {
                auto added_component = std::dynamic_pointer_cast<T>(component);

                if (added_component)
                {
                    ptr = added_component;
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}
