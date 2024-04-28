#pragma once

#include "Scene.h"
#include "Transform.h"
#include "AK/Types.h"
#include "AK/Badge.h"

#include <array>

class Camera;

namespace Editor
{

enum class GuizmoOperationType
{
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

enum class AssetType
{
    Unknown,
    Model,
};

class Asset
{
public:
    Asset(std::string const& path, AssetType const type) : path(path), type(type)
    {
    }

    std::string path;
    AssetType type;
};

class Editor
{
public:
    static std::shared_ptr<Editor> create();

    explicit Editor(AK::Badge<Editor>);

    void draw();
    void set_scene(std::shared_ptr<Scene> const& scene);
    void handle_input();
    void set_docking_space() const;

private:
    void switch_rendering_to_editor();

    void draw_debug_window();
    void draw_content_browser();
    void draw_game();
    void draw_inspector();
    void draw_scene_hierarchy();
    void draw_scene_save() const;

    void draw_entity_recursively(std::shared_ptr<Transform> const& transform);
    bool draw_entity_popup(std::shared_ptr<Entity> const& entity);

    void load_assets();
    void save_scene() const;
    bool load_scene() const;
    void set_style() const;

    void camera_input() const;
    void non_camera_input();

    void delete_selected_entity() const;

    void mouse_callback(double const x, double const y);

    glm::dvec2 m_last_mouse_position = glm::dvec2(1280.0 / 2.0, 720.0 / 2.0);
    float m_yaw = 0.0f;
    float m_pitch = 10.0f;
    bool m_mouse_just_entered = true;
    double m_sensitivity = 0.1;
    float m_camera_speed = 12.5f;

    std::shared_ptr<Entity> m_camera_entity;
    std::shared_ptr<Camera> m_editor_camera;
    std::shared_ptr<Camera> m_scene_camera;

    std::vector<Asset> m_assets = {};

    glm::vec2 m_game_position = {};
    glm::vec2 m_game_size = {};

    std::weak_ptr<Entity> m_selected_entity;
    std::shared_ptr<Scene> m_open_scene;

    EditorWindow m_debug_window = {};
    EditorWindow m_content_browser_window = {};
    EditorWindow m_game_window = {};
    EditorWindow m_inspector_window = {};
    EditorWindow m_hierarchy_window = {};

    bool m_polygon_mode_active = false;
    i64 m_frame_count = 0;
    double m_current_time = 0.0;
    double m_last_second = 0.0;
    double m_average_ms_per_frame = 0.0;

    bool m_rendering_to_editor = true;

    GuizmoOperationType m_operation_type = GuizmoOperationType::Translate;

    std::string content_path = "./res/";
    std::array<std::string, 3> m_known_model_formats = { ".gltf", ".obj", ".fbx" };
};

}
