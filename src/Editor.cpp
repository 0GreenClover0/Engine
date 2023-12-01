#include "Editor.h"

#include <imgui.h>

#include "Entity.h"
#include "MainScene.h"

namespace Engine
{

void Editor::draw_scene_hierarchy() const
{
    ImGui::Begin("Hierarchy");

    // Draw every entity without a parent, and draw its children recursively
    for (auto const& entity : MainScene::get_instance()->entities)
    {
        if (!entity->transform->parent.expired())
            continue;

        draw_entity_recursively(entity->transform);
    }
    ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform) const
{
    if (auto const entity = transform->entity.lock(); !ImGui::TreeNode(reinterpret_cast<void*>(static_cast<intptr_t>(entity->hashed_guid)), "%s", entity->name))
    {
        return;
    }

    for (auto const& child : transform->children)
    {
        draw_entity_recursively(child);
    }

    ImGui::TreePop();
}

}
