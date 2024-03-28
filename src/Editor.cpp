#include "Editor.h"

#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

#include "Entity.h"
#include "SceneSerializer.h"

namespace Editor
{
void Editor::set_scene(std::shared_ptr<Scene> const& scene)
{
    m_open_scene = scene;
}

void Editor::draw_debug_window(bool* debug_open, int const window_flags, bool* polygon_mode, double const frame_per_second) const
{
    ImGui::Begin("Debug", debug_open, window_flags);
    ImGui::Checkbox("Polygon mode", polygon_mode);
    ImGui::Text("Application average %.3f ms/frame", frame_per_second);
    draw_scene_save();
    ImGui::End();
}

void Editor::draw_scene_hierarchy()
{
    ImGui::Begin("Hierarchy");

    // Draw every entity without a parent, and draw its children recursively
    for (auto const& entity : m_open_scene->entities)
    {
        if (!entity->transform->parent.expired())
            continue;

        draw_entity_recursively(entity->transform);
    }
    ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform)
{
    auto const entity = transform->entity.lock();
    ImGuiTreeNodeFlags const node_flags = (!m_selected_entity.expired() && m_selected_entity.lock()->hashed_guid == entity->hashed_guid ? ImGuiTreeNodeFlags_Selected : 0) | (
        transform->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;

    if (!ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity->hashed_guid)), node_flags, "%s", entity->name.c_str()))
    {
        if (ImGui::IsItemClicked())
            m_selected_entity = entity;

        return;
    }

    if (ImGui::IsItemClicked())
        m_selected_entity = entity;

    for (auto const& child : transform->children)
    {
        draw_entity_recursively(child);
    }

    ImGui::TreePop();
}

void Editor::draw_inspector() const
{
    ImGui::Begin("Inspector");

    if (m_selected_entity.expired())
    {
        ImGui::End();
        return;
    }

    auto const entity = m_selected_entity.lock();

    ImGui::Text("Transform");
    ImGui::Spacing();

    float position[] = { entity->transform->get_local_position().x, entity->transform->get_local_position().y, entity->transform->get_local_position().z };
    ImGui::InputFloat3("Position", position);

    auto const new_position = glm::vec3(position[0], position[1], position[2]);
    entity->transform->set_local_position(new_position);

    float rotation[] = { entity->transform->get_euler_angles().x, entity->transform->get_euler_angles().y, entity->transform->get_euler_angles().z };
    ImGui::InputFloat3("Rotation", rotation);

    auto const new_rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);
    entity->transform->set_euler_angles(new_rotation);

    float scale[] = { entity->transform->get_local_scale().x, entity->transform->get_local_scale().y, entity->transform->get_local_scale().z };
    ImGui::InputFloat3("Scale", scale);

    auto const new_scale = glm::vec3(scale[0], scale[1], scale[2]);
    entity->transform->set_local_scale(new_scale);

    for (auto const& component : entity->components)
    {
        ImGui::Text(component->get_name().c_str());
        ImGui::Spacing();

        ImGui::Checkbox("Enabled", &component->enabled);

        component->draw_editor();

        ImGui::Spacing();
    }

    ImGui::End();
}

void Editor::draw_scene_save() const
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save scene"))
            {
                save_scene();
            }

            if (ImGui::MenuItem("Load scene"))
            {
                load_scene();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Editor::save_scene() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    scene_serializer->serialize("./res/scenes/scene.txt");
}

bool Editor::load_scene() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);

    return scene_serializer->deserialize("./res/scenes/scene.txt");
}

}
