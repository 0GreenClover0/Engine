#pragma once

#include "Scene.h"
#include "Transform.h"

namespace Editor
{

enum class GuizmoOperationType {
    Translate,
    Rotate,
    Scale,
    None
};

class Editor
{
public:
    Editor() = default;
    explicit Editor(std::shared_ptr<Scene> const& scene) : m_open_scene(scene) { }

    void set_scene(std::shared_ptr<Scene> const& scene);
    void draw_debug_window(bool* debug_open, int const window_flags, bool* polygon_mode, double const frame_per_second) const;
    void draw_inspector() const;
    void draw_scene_hierarchy();
    void draw_scene_save() const;
    void handle_input();

private:
    void draw_entity_recursively(std::shared_ptr<Transform> const& transform);
    void save_scene() const;
    bool load_scene() const;

    std::weak_ptr<Entity> m_selected_entity;
    std::shared_ptr<Scene> m_open_scene;

    GuizmoOperationType m_operation_type = GuizmoOperationType::Translate;
};

}
