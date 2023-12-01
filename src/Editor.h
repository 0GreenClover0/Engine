#pragma once

#include "Transform.h"

namespace Engine
{

class Editor
{
public:
    void draw_scene_hierarchy() const;

private:
    void draw_entity_recursively(std::shared_ptr<Transform> const& transform) const;
};

}
