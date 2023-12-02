#pragma once

#include "Scene.h"
#include "Transform.h"

namespace Editor
{

class Editor
{
public:
    Editor() = default;
    explicit Editor(std::shared_ptr<Scene> const& scene) : open_scene(scene) { }

    void set_scene(std::shared_ptr<Scene> const& scene);
    void draw_scene_hierarchy() const;
    void draw_scene_save() const;

private:
    void draw_entity_recursively(std::shared_ptr<Transform> const& transform) const;
    void save_scene() const;
    bool load_scene() const;

    std::shared_ptr<Scene> open_scene;
};

}
