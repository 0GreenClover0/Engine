#pragma once

#include "Scene.h"
#include "Transform.h"
#include "AK/Types.h"

namespace Editor
{

enum class GuizmoOperationType {
    Translate,
    Rotate,
    Scale,
    None
};

class EditorWindow
{
public:
    bool open = true;
    i32 flags = 0;
};

class Editor
{
public:
    Editor() = default;
    explicit Editor(std::shared_ptr<Scene> const& scene);

    void draw();
    void set_scene(std::shared_ptr<Scene> const& scene);
    void handle_input();
    void set_docking_space() const;

private:
    void draw_debug_window();
    void draw_inspector();
    void draw_scene_hierarchy();
    void draw_scene_save() const;

    void draw_entity_recursively(std::shared_ptr<Transform> const& transform);
    void save_scene() const;
    bool load_scene() const;
    void set_style() const;

    std::weak_ptr<Entity> m_selected_entity;
    std::shared_ptr<Scene> m_open_scene;

    EditorWindow m_debug_window = {};
    EditorWindow m_inspector_window = {};
    EditorWindow m_hierarchy_window = {};

    bool m_polygon_mode_active = false;
    i64 m_frame_count = 0;
    double m_current_time = 0.0;
    double m_last_second = 0.0;
    double m_average_ms_per_frame = 0.0;

    GuizmoOperationType m_operation_type = GuizmoOperationType::Translate;
};

}
