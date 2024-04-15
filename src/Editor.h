#pragma once

#include "Scene.h"
#include "Transform.h"
#include "AK/Types.h"
#include "AK/Badge.h"

#include <array>

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

    void load_assets();
    void save_scene() const;
    bool load_scene() const;
    void set_style() const;

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
