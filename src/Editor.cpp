#include "Editor.h"

#include <imgui.h>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "Entity.h"
#include "SceneSerializer.h"

namespace Editor
{
void Editor::set_scene(std::shared_ptr<Scene> const& scene)
{
    this->open_scene = scene;
}

void Editor::draw_scene_hierarchy() const
{
    ImGui::Begin("Hierarchy");

    // Draw every entity without a parent, and draw its children recursively
    for (auto const& entity : open_scene->entities)
    {
        if (!entity->transform->parent.expired())
            continue;

        draw_entity_recursively(entity->transform);
    }
    ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform) const
{
    if (auto const entity = transform->entity.lock(); !ImGui::TreeNode(reinterpret_cast<void*>(static_cast<intptr_t>(entity->hashed_guid)), "%s", entity->name.c_str()))
    {
        return;
    }

    for (auto const& child : transform->children)
    {
        draw_entity_recursively(child);
    }

    ImGui::TreePop();
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
    auto const scene_serializer = std::make_shared<SceneSerializer>(open_scene);
    scene_serializer->serialize("./res/scenes/scene.txt");
}

bool Editor::load_scene() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(open_scene);

    return scene_serializer->deserialize("./res/scenes/scene.txt");
}

}
