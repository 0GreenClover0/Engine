#include "Editor.h"

#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

#include "Entity.h"
#include "Camera.h"
#include "SceneSerializer.h"
#include "Engine.h"
#include "Input.h"

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

    auto const camera = Camera::get_main_camera();
    auto const entity = m_selected_entity.lock();

    ImGui::Text("Transform");
    ImGui::Spacing();

    glm::vec3 position = entity->transform->get_local_position();
    ImGui::InputFloat3("Position", glm::value_ptr(position));
    entity->transform->set_local_position(position);

    glm::vec3 rotation = entity->transform->get_euler_angles();
    ImGui::InputFloat3("Rotation", glm::value_ptr(rotation));
    entity->transform->set_euler_angles(rotation);

    glm::vec3 scale = entity->transform->get_local_scale();
    ImGui::InputFloat3("Scale", glm::value_ptr(scale));
    entity->transform->set_local_scale(scale);

    for (auto const& component : entity->components)
    {
        ImGui::Text(component->get_name().c_str());
        ImGui::Spacing();

        ImGui::Checkbox("Enabled", &component->enabled);

        component->draw_editor();

        ImGui::Spacing();
    }

    // GIZMOS CODE
    ImGuiIO const& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

    bool was_transform_changed = false;
    glm::mat4 global_model = entity->transform->get_model_matrix();
    switch (m_operation_type)
    {
    case GuizmoOperationType::Translate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()), ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(global_model), nullptr, nullptr);
        break;
    case GuizmoOperationType::Scale:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()), ImGuizmo::OPERATION::SCALE, ImGuizmo::MODE::LOCAL, glm::value_ptr(global_model), nullptr, nullptr);
        break;
    case GuizmoOperationType::Rotate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()), ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::LOCAL, glm::value_ptr(global_model), nullptr, nullptr);
        break;
    case GuizmoOperationType::None:
    default:
        break;
    }

    if (was_transform_changed)
    {
        glm::mat4 local = global_model;

        auto const parent = entity->transform->parent.lock();
        if (parent != nullptr)
        {
            local = glm::inverse(parent->get_model_matrix()) * local;
        }

        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local), glm::value_ptr(position), glm::value_ptr(rotation), glm::value_ptr(scale));
        if (m_operation_type == GuizmoOperationType::Translate)
        {
            entity->transform->set_local_position(position);
        }
        else if (m_operation_type == GuizmoOperationType::Rotate)
        {
            entity->transform->set_euler_angles(rotation);
        }
        else if (m_operation_type == GuizmoOperationType::Scale)
        {
            entity->transform->set_local_scale(scale);
        }
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

void Editor::handle_input()
{
    auto const input = Input::input;

    if (input->get_key_down(GLFW_KEY_W))
    {
        m_operation_type = GuizmoOperationType::Translate;
    }

    if (input->get_key_down(GLFW_KEY_R))
    {
        m_operation_type = GuizmoOperationType::Scale;
    }

    if (input->get_key_down(GLFW_KEY_E))
    {
        m_operation_type = GuizmoOperationType::Rotate;
    }

    if (input->get_key_down(GLFW_KEY_G))
    {
        m_operation_type = GuizmoOperationType::None;
    }
}

void Editor::set_docking_space() const
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

}
