#include "Editor.h"

#include <imgui.h>

#include "Entity.h"
#include "MainScene.h"

namespace Engine
{

void Editor::draw_scene_hierarchy() const
{
	ImGui::Begin("Hierarchy");
	int current_item = 1;
	char t = 'a';

	int tree_index = 0;
	for (auto const& entity : MainScene::get_instance()->entities)
	{
		if (!entity->transform->parent.expired())
			continue;

        draw_entity_recursively(entity->transform, tree_index);
	}
	ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform, int& tree_id) const
{
	if (ImGui::TreeNode(reinterpret_cast<void*>(static_cast<intptr_t>(tree_id)), "%s", transform->entity.lock()->name))
    {
		tree_id++;

		for (auto const& child : transform->children)
		{
			draw_entity_recursively(child, tree_id);
		}

        ImGui::TreePop();
    }
	else
	{
		tree_id++;
	}
}

}
