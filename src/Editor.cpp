#include "Editor.h"

#include "imgui_extensions.h"
#include "imgui_stdlib.h"
#include <ImGuizmo.h>
#include <filesystem>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/string_cast.hpp>
#include <imgui.h>

#include "Camera.h"
#include "Collider2D.h"
#include "ComponentList.h"
#include "Cube.h"
#include "Curve.h"
#include "Debug.h"
#include "DebugDrawing.h"
#include "DirectionalLight.h"
#include "Ellipse.h"
#include "Engine.h"
#include "Entity.h"
#include "ExampleDynamicText.h"
#include "ExampleUIBar.h"
#include "Game/Factory.h"
#include "Game/LevelController.h"
#include "Game/Lighthouse.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "Game/Path.h"
#include "Game/Player.h"
#include "Game/Player/PlayerInput.h"
#include "Game/Port.h"
#include "Game/Ship.h"
#include "Game/ShipSpawner.h"
#include "Globals.h"
#include "Grass.h"
#include "Input.h"
#include "Light.h"
#include "Model.h"
#include "PointLight.h"
#include "RendererDX11.h"
#include "SceneSerializer.h"
#include "ScreenText.h"
#include "Sound.h"
#include "SoundListener.h"
#include "Sphere.h"
#include "SpotLight.h"
#include "Sprite.h"
#include "Water.h"
// # Put new header here

namespace Editor
{

Editor::Editor(AK::Badge<Editor>)
{
    set_style();

    add_debug_window();
    add_content_browser();
    add_game();
    add_inspector();
    add_scene_hierarchy();

    m_last_second = glfwGetTime();

    load_assets();

    m_camera_entity = Entity::create_internal("Editor Camera");
    m_editor_camera = m_camera_entity->add_component_internal<Camera>(Camera::create());
    reset_camera();

    if (!Engine::is_game_running())
        Camera::set_main_camera(m_editor_camera);
}

Editor::~Editor()
{
    std::filesystem::path const copied_entity_path = m_copied_entity_path;

    if (std::filesystem::exists(copied_entity_path))
    {
        std::filesystem::remove(copied_entity_path);
    }
}

std::shared_ptr<Editor> Editor::create()
{
    auto editor = std::make_shared<Editor>(AK::Badge<Editor> {});

    Input::input->on_set_cursor_pos_event.attach(&Editor::mouse_callback, editor);

    return editor;
}

void Editor::draw()
{
    if (!m_rendering_to_editor)
        return;

    auto const windows_copy = m_editor_windows;
    for (auto& window : windows_copy)
    {
        switch (window->type)
        {
        case EditorWindowType::Debug:
            draw_debug_window(window);
            break;
        case EditorWindowType::Content:
            draw_content_browser(window);
            break;
        case EditorWindowType::Hierarchy:
            draw_scene_hierarchy(window);
            break;
        case EditorWindowType::Game:
            draw_game(window);
            break;
        case EditorWindowType::Inspector:
            draw_inspector(window);
            break;
        case EditorWindowType::Custom:
            std::cout << "Custom Editor windows are currently not supported.\n";
            break;
        }
    }
}

void Editor::set_scene(std::shared_ptr<Scene> const& scene)
{
    m_open_scene = scene;
}

void Editor::draw_debug_window(std::shared_ptr<EditorWindow> const& window)
{
    m_current_time = glfwGetTime();
    m_frame_count += 1;

    if (m_current_time - m_last_second >= 1.0)
    {
        m_average_ms_per_frame = 1000.0 / static_cast<double>(m_frame_count);
        m_frame_count = 0;
        m_last_second = glfwGetTime();
    }

    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    ImGui::Checkbox("Polygon mode", &m_polygon_mode_active);
    ImGui::Text("Application average %.3f ms/frame", m_average_ms_per_frame);
    draw_scene_save();

    std::string const log_count = "Logs " + std::to_string(Debug::debug_messages.size());
    ImGui::Text(log_count.c_str());
    if (ImGui::Button("Clear log"))
    {
        Debug::clear();
    }
    if (ImGui::BeginListBox("Logs", ImVec2(-FLT_MIN, 0.0f)))
    {
        ImGuiListClipper clipper;
        clipper.Begin(Debug::debug_messages.size());
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                switch (Debug::debug_messages[i].type)
                {
                case DebugType::Log:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    break;
                case DebugType::Warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 250, 0, 255));
                    break;
                case DebugType::Error:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 55, 0, 255));
                    break;
                default:
                    std::unreachable();
                }

                ImGui::Text(Debug::debug_messages[i].text.c_str());
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndListBox();
    }

    ImGui::End();

    Renderer::get_instance()->wireframe_mode_active = m_polygon_mode_active;
}

void Editor::draw_content_browser(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    for (auto const& asset : m_assets)
    {
        if (ImGui::Selectable(asset.path.c_str()))
        {
            ImGui::SetClipboardText(asset.path.c_str());
        }
    }

    ImGui::End();
}

void Editor::draw_game(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool open = false;
    if (Engine::is_game_running())
    {
        window->set_name("Game");
        open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);
    }
    else
    {
        window->set_name("Scene");
        open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);
    }

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (ImGui::BeginMenuBar())
    {
        bool const is_game_running = Engine::is_game_running();
        if (is_game_running)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(120, 120, 0, 150));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 50));

            if (ImGui::BeginMenu("Camera"))
            {
                float fov_angle = glm::degrees(Camera::get_main_camera()->fov);
                ImGui::SliderFloat("Camera FoV", &fov_angle, 1.0f, 90.0f);
                Camera::get_main_camera()->fov = glm::radians(fov_angle);

                if (ImGui::Button("Reset camera"))
                {
                    reset_camera();
                }

                ImGui::EndMenu();
            }

            ImVec4 constexpr active_button = {0.2f, 0.5f, 0.4f, 1.0f};
            ImVec4 constexpr inactive_button = {0.05f, 0.05f, 0.05f, 0.54f};

            if (m_gizmo_snapping)
                ImGui::PushStyleColor(ImGuiCol_Button, active_button);
            else
                ImGui::PushStyleColor(ImGuiCol_Button, inactive_button);

            if (ImGui::Button("Gizmo snapping"))
            {
                switch_gizmo_snapping();
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("Shortcut: \\");
            }

            ImGui::PopStyleColor();
        }

        if (ImGui::Button("Play", ImVec2(50.0f, 20.0f)))
        {
            if (is_game_running)
            {
                m_open_scene = nullptr;
            }

            Engine::set_game_running(!is_game_running);

            if (is_game_running)
            {
                m_open_scene = MainScene::get_instance();
                Camera::set_main_camera(m_editor_camera);
            }
            else
            {
                Renderer::get_instance()->choose_main_camera(m_editor_camera);
            }
        }

        ImGui::PopStyleColor();

        ImGui::EndMenuBar();
    }

    auto vec2 = ImGui::GetContentRegionAvail();
    m_game_size = {vec2.x, vec2.y};

    vec2 = ImGui::GetWindowPos();
    m_game_position = {vec2.x, vec2.y};

    if (Renderer::renderer_api == Renderer::RendererApi::DirectX11)
    {
        ImGui::Image(RendererDX11::get_instance_dx11()->get_render_texture_view(), ImVec2(m_game_size.x, m_game_size.y));
    }

    if (m_selected_entity.expired())
    {
        ImGui::End();
        return;
    }

    auto const camera = Camera::get_main_camera();
    auto const entity = m_selected_entity.lock();

    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(m_game_position.x, m_game_position.y, m_game_size.x, m_game_size.y);

    bool was_transform_changed = false;
    glm::mat4 global_model = entity->transform->get_model_matrix();
    switch (m_operation_type)
    {
    case GuizmoOperationType::Translate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_position_snap) : nullptr);
        break;
    case GuizmoOperationType::Scale:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::SCALE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_scale_snap) : nullptr);
        break;
    case GuizmoOperationType::Rotate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_rotation_snap) : nullptr);
        break;
    case GuizmoOperationType::None:
    default:
        break;
    }

    if (was_transform_changed)
    {
        entity->transform->set_model_matrix(global_model);
    }

    ImGui::End();
}

void Editor::draw_scene_hierarchy(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("Add Entity"))
        {
            Entity::create("Entity");
        }

        ImGui::EndMenuBar();
    }

    // Draw every entity without a parent, and draw its children recursively
    auto const entities_copy = m_open_scene->entities;
    for (auto const& entity : entities_copy)
    {
        if (!entity->transform->parent.expired())
            continue;

        draw_entity_recursively(entity->transform);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)
        && !ImGui::IsAnyItemHovered())
    {
        ImGui::OpenPopup("HierarchyPopup");
    }

    if (ImGui::BeginPopup("HierarchyPopup", ImGuiPopupFlags_MouseButtonRight))
    {
        std::filesystem::path const copied_entity_path = m_copied_entity_path;
        bool const copied_entity_exists = std::filesystem::exists(copied_entity_path);

        if (!copied_entity_exists)
        {
            ImGui::BeginDisabled(true);
        }

        if (ImGui::Button("Paste"))
        {
            paste_entity();
            ImGui::CloseCurrentPopup();
        }

        if (!copied_entity_exists)
        {
            ImGui::EndDisabled();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform)
{
    auto const entity = transform->entity.lock();
    ImGuiTreeNodeFlags const node_flags =
        (!m_selected_entity.expired() && m_selected_entity.lock()->hashed_guid == entity->hashed_guid ? ImGuiTreeNodeFlags_Selected : 0)
        | (transform->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_OpenOnArrow;

    if (!ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity->hashed_guid)), node_flags, "%s", entity->name.c_str()))
    {
        if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            m_selected_entity = entity;
        }

        if (!draw_entity_popup(entity))
        {
            return;
        }

        entity_drag(entity);

        return;
    }

    entity_drag(entity);

    if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_selected_entity = entity;
    }

    if (!draw_entity_popup(entity))
    {
        ImGui::TreePop();
        return;
    }

    for (auto const& child : transform->children)
    {
        draw_entity_recursively(child);
    }

    ImGui::TreePop();
}

// Should be called just after an Entity item
void Editor::entity_drag(std::shared_ptr<Entity> const& entity)
{
    ImGuiDragDropFlags src_flags = 0;
    src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;

    if (ImGui::BeginDragDropSource(src_flags))
    {
        ImGui::Text((entity->name).c_str());
        ImGui::SetDragDropPayload("guid", entity->guid.data(), sizeof(i64) * 8);
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        std::string guid;

        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("guid"))
        {
            guid.resize(sizeof(i64) * 8);
            memcpy(guid.data(), payload->Data, sizeof(i64) * 8);

            if (auto const reparent_entity = MainScene::get_instance()->get_entity_by_guid(guid))
            {
                reparent_entity->transform->set_parent(entity->transform);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

bool Editor::draw_entity_popup(std::shared_ptr<Entity> const& entity)
{
    if (!m_selected_entity.expired() && ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
    {
        if (m_selected_entity.lock() != entity)
        {
            m_selected_entity = entity;
        }

        if (ImGui::Button("Rename"))
        {
            ImGui::OpenPopup("RenamePopup");
        }

        if (ImGui::BeginPopup("RenamePopup"))
        {
            if (ImGui::InputText("##empty", &entity->name, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Delete"))
        {
            delete_selected_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }

        if (ImGui::Button("Copy"))
        {
            copy_selected_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        if (ImGui::Button("Add child"))
        {
            add_child_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        ImGui::EndPopup();
    }

    return true;
}

void Editor::draw_window_menu_bar(std::shared_ptr<EditorWindow> const& window)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::BeginMenu("Add window"))
            {
                if (ImGui::MenuItem("Inspector"))
                {
                    add_inspector();
                }
                if (ImGui::MenuItem("Game"))
                {
                    add_game();
                }
                if (ImGui::MenuItem("Content"))
                {
                    add_content_browser();
                }
                if (ImGui::MenuItem("Hierarchy"))
                {
                    add_scene_hierarchy();
                }
                if (ImGui::MenuItem("Debug"))
                {
                    add_debug_window();
                }

                ImGui::EndMenu();
            }

            if (window->type == EditorWindowType::Inspector)
            {
                if (ImGui::Button("Lock"))
                {
                    window->set_is_locked(!window->is_locked(), LockData {m_selected_entity});
                }
            }

            ImGui::EndMenu();
        }

        if (window->is_locked())
        {
            ImGui::Text("LOCKED");
        }

        ImGui::EndMenuBar();
    }
}

void Editor::load_assets()
{
    for (auto const& entry : std::filesystem::recursive_directory_iterator(content_path))
    {
        if (std::ranges::find(m_known_model_formats, entry.path().extension().string()) != m_known_model_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Model);
        }

        // TODO: Load other assets...
    }
}

void Editor::draw_inspector(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (window->is_locked() && window->get_locked_entity().expired())
    {
        window->set_is_locked(false, {});
    }

    std::weak_ptr<Entity> current_entity = window->get_locked_entity();

    if (current_entity.expired())
    {
        current_entity = m_selected_entity;
    }

    if (current_entity.expired())
    {
        ImGui::End();
        return;
    }

    auto const camera = Camera::get_main_camera();
    auto const entity = current_entity.lock();

    ImGui::Text("Transform");
    ImGui::Spacing();

    glm::vec3 position = entity->transform->get_local_position();
    ImGuiEx::InputFloat3("Position", glm::value_ptr(position));

    float const input_width = ImGui::CalcItemWidth() / 3.0f - ImGui::GetStyle().ItemSpacing.x * 0.66f;

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (ImGui::Button("Copy##1"))
    {
        std::string const cpy = glm::to_string(position);
        ImGui::SetClipboardText(cpy.c_str());
    }

    entity->transform->set_local_position(position);

    glm::vec3 rotation = entity->transform->get_euler_angles();
    ImGuiEx::InputFloat3("Rotation", glm::value_ptr(rotation));
    entity->transform->set_euler_angles(rotation);

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (ImGui::Button("Copy##2"))
    {
        std::string const cpy = glm::to_string(rotation);
        ImGui::SetClipboardText(cpy.c_str());
    }

    glm::vec3 scale = entity->transform->get_local_scale();
    glm::vec3 old_scale = scale;

    ImGui::PushItemWidth(input_width);

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.x);
    ImGuiEx::InputFloat("##x", &scale.x);
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.y);
    ImGuiEx::InputFloat("##y", &scale.y);
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.z);
    ImGuiEx::InputFloat("Scale##z", &scale.z);
    ImGui::EndDisabled();

    ImGui::PopItemWidth();

    if (scale != old_scale && m_lock_scale)
    {
        scale = update_locked_value(scale, old_scale);
    }

    entity->transform->set_local_scale(scale);

    ImGui::SameLine();
    ImGui::Text("    | ");
    ImGui::SameLine();

    if (ImGui::Button("Copy##3"))
    {
        std::string const cpy = glm::to_string(scale);
        ImGui::SetClipboardText(cpy.c_str());
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("LOCK", &m_lock_scale))
    {
        if (!m_lock_scale)
        {
            m_disabled_scale = {false, false, false};
        }
        else
        {
            m_lock_ratio = entity->transform->get_local_scale();

            m_disabled_scale.x = glm::epsilonEqual(m_lock_ratio.x, 0.0f, 0.0001f);

            m_disabled_scale.y = glm::epsilonEqual(m_lock_ratio.y, 0.0f, 0.0001f);

            m_disabled_scale.z = glm::epsilonEqual(m_lock_ratio.z, 0.0f, 0.0001f);
        }
    }

    auto const components_copy = entity->components;
    for (auto const& component : components_copy)
    {
        ImGui::Spacing();
        std::string guid = "##" + component->guid;

        // NOTE: This only returns unmangled name while using the MSVC compiler
        std::string const typeid_name = typeid(*component).name();
        std::string const name = typeid_name.substr(6) + " " + component->custom_name;

        bool const component_open = ImGui::TreeNode((name + guid).c_str());

        ImGuiDragDropFlags src_flags = 0;
        src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;

        if (ImGui::BeginDragDropSource(src_flags))
        {
            ImGui::Text((entity->name + " : " + name).c_str());
            ImGui::SetDragDropPayload("guid", component->guid.data(), sizeof(i64) * 8);
            ImGui::EndDragDropSource();
        }

        ImGui::Spacing();

        if (component_open)
        {
            bool enabled = component->enabled();
            ImGui::Checkbox("Enabled", &enabled);
            component->set_enabled(enabled);

            component->draw_editor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TreePop();
        }
    }

    if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, -FLT_MIN)))
    {
        ImGui::Text("Search bar");

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 5));
        ImGui::InputText("##filter", &m_search_filter);
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();

        std::ranges::transform(m_search_filter, m_search_filter.begin(), [](u8 const c) { return std::tolower(c); });

#define CONCAT_CLASS(name) class name
#define ENUMERATE_COMPONENT(name, ui_name)                                                                        \
    {                                                                                                             \
        std::string ui_name_lower(ui_name);                                                                       \
        std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); }); \
        if (m_search_filter.empty() || ui_name_lower.find(m_search_filter) != std::string::npos)                  \
        {                                                                                                         \
            if (ImGui::Button(ui_name, ImVec2(-FLT_MIN, 20)))                                                     \
                entity->add_component<CONCAT_CLASS(name)>(##name::create());                                      \
        }                                                                                                         \
    }
        ENUMERATE_COMPONENTS
#undef ENUMERATE_COMPONENT

        ImGui::EndListBox();
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
                if (Engine::is_game_running())
                {
                    Debug::log("Game is currently running. Scene has not been saved.", DebugType::Error);
                }
                else
                {
                    save_scene();
                }
            }

            if (ImGui::MenuItem("Load scene"))
            {
                MainScene::get_instance()->unload();

                bool const loaded = load_scene();

                if (!loaded)
                {
                    Debug::log("Could not load a scene.", DebugType::Error);
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Editor::save_scene() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    scene_serializer->set_instance(scene_serializer);
    scene_serializer->serialize("./res/scenes/scene.txt");
}

bool Editor::load_scene() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    scene_serializer->set_instance(scene_serializer);
    return scene_serializer->deserialize("./res/scenes/scene.txt");
}

void Editor::set_style() const
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

void Editor::camera_input() const
{
    float const current_speed = m_camera_speed * delta_time;

    if (Input::input->get_key(GLFW_KEY_W))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       current_speed * m_editor_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_S))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       current_speed * m_editor_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_A))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       glm::normalize(glm::cross(m_editor_camera->get_front(), m_editor_camera->get_up()))
                                                       * current_speed);

    if (Input::input->get_key(GLFW_KEY_D))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       glm::normalize(glm::cross(m_editor_camera->get_front(), m_editor_camera->get_up()))
                                                       * current_speed);

    if (Input::input->get_key(GLFW_KEY_E))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       current_speed * glm::vec3(0.0f, 1.0f, 0.0f));

    if (Input::input->get_key(GLFW_KEY_Q))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       current_speed * glm::vec3(0.0f, 1.0f, 0.0f));
}

void Editor::non_camera_input()
{
    if (Input::input->get_key_down(GLFW_KEY_W))
    {
        m_operation_type = GuizmoOperationType::Translate;
    }

    if (Input::input->get_key_down(GLFW_KEY_R))
    {
        m_operation_type = GuizmoOperationType::Scale;
    }

    if (Input::input->get_key_down(GLFW_KEY_E))
    {
        m_operation_type = GuizmoOperationType::Rotate;
    }

    if (Input::input->get_key_down(GLFW_KEY_G))
    {
        m_operation_type = GuizmoOperationType::None;
    }

    if (Input::input->get_key_down(GLFW_KEY_BACKSLASH))
    {
        switch_gizmo_snapping();
    }
}

void Editor::reset_camera()
{
    m_camera_entity->transform->set_local_position(m_camera_default_position);
    m_camera_entity->transform->set_euler_angles(m_camera_default_rotation);
    m_editor_camera->set_fov(glm::radians(m_camera_default_fov));
}

void Editor::switch_gizmo_snapping()
{
    m_gizmo_snapping = !m_gizmo_snapping;

    if (m_selected_entity.expired())
        return;

    auto const entity = m_selected_entity.lock();

    // FIXME: We should probably run this when selected entity changes as well.
    switch (m_operation_type)
    {
    case GuizmoOperationType::Translate:
        entity->transform->set_local_position(glm::round(entity->transform->get_local_position() * 100.0f) / 100.0f);
        break;
    case GuizmoOperationType::Scale:
        entity->transform->set_local_scale(glm::round(entity->transform->get_local_position() * 100.0f) / 100.0f);
        break;
    case GuizmoOperationType::Rotate:
        entity->transform->set_euler_angles(glm::round(entity->transform->get_euler_angles()));
        break;
    case GuizmoOperationType::None:
    default:
        break;
    }
}

void Editor::delete_selected_entity() const
{
    if (!m_selected_entity.expired())
    {
        m_selected_entity.lock()->destroy_immediate();
    }
}

void Editor::copy_selected_entity() const
{
    if (m_selected_entity.expired())
        return;

    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    scene_serializer->set_instance(scene_serializer);
    scene_serializer->serialize_this_entity(m_selected_entity.lock(), m_copied_entity_path);
}

void Editor::paste_entity() const
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    scene_serializer->set_instance(scene_serializer);
    scene_serializer->deserialize_this_entity("./.editor/copied_entity.txt");
}

void Editor::add_child_entity() const
{
    if (m_selected_entity.expired())
        return;

    auto const entity = m_selected_entity.lock();
    auto const child_entity = Entity::create("Child");
    child_entity->transform->set_parent(entity->transform);
}

void Editor::mouse_callback(double const x, double const y)
{
    if (m_mouse_just_entered)
    {
        m_last_mouse_position.x = x;
        m_last_mouse_position.y = y;
        m_mouse_just_entered = false;
    }

    double x_offset = x - m_last_mouse_position.x;
    double y_offset = m_last_mouse_position.y - y;
    m_last_mouse_position.x = x;
    m_last_mouse_position.y = y;

    if (!Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
        return;

    x_offset *= m_sensitivity;
    y_offset *= m_sensitivity;

    m_yaw += x_offset;
    m_pitch = glm::clamp(m_pitch + y_offset, -89.0, 89.0);

    m_camera_entity->transform->set_euler_angles(glm::vec3(m_pitch, -m_yaw, 0.0f));
}

glm::vec3 Editor::update_locked_value(glm::vec3 new_value, glm::vec3 const old_value) const
{
    if (glm::epsilonNotEqual(new_value.x, old_value.x, 0.0001f))
    {
        if (m_disabled_scale.x)
        {
            return old_value;
        }

        float const y = (m_lock_ratio.y / m_lock_ratio.x) * new_value.x;
        float const z = (m_lock_ratio.z / m_lock_ratio.x) * new_value.x;

        return {new_value.x, y, z};
    }

    if (glm::epsilonNotEqual(new_value.y, old_value.y, 0.0001f))
    {
        if (m_disabled_scale.y)
        {
            return old_value;
        }

        float const x = (m_lock_ratio.x / m_lock_ratio.y) * new_value.y;
        float const z = (m_lock_ratio.z / m_lock_ratio.y) * new_value.y;

        return {x, new_value.y, z};
    }

    if (glm::epsilonNotEqual(new_value.z, old_value.z, 0.0001f))
    {
        if (m_disabled_scale.z)
        {
            return old_value;
        }

        float const x = (m_lock_ratio.x / m_lock_ratio.z) * new_value.z;
        float const y = (m_lock_ratio.y / m_lock_ratio.z) * new_value.z;

        return {x, y, new_value.z};
    }

    return new_value;
}

void Editor::handle_input()
{
    auto const input = Input::input;

    if (input->get_key_down(GLFW_KEY_F1))
    {
        switch_rendering_to_editor();
    }

    if (input->get_key_down(GLFW_KEY_F5))
    {
        Renderer::get_instance()->reload_shaders();
    }

    if (input->get_key_down(GLFW_KEY_DELETE))
    {
        if (!ImGui::IsAnyItemActive())
        {
            delete_selected_entity();
        }
    }

    if (!input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
    {
        non_camera_input();
    }
    else
    {
        camera_input();
    }
}

void Editor::set_docking_space() const
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void Editor::switch_rendering_to_editor()
{
    Renderer::get_instance()->switch_rendering_to_texture();
    m_rendering_to_editor = !m_rendering_to_editor;
}

void Editor::add_debug_window()
{
    auto debug_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Debug);
    m_editor_windows.emplace_back(debug_window);
}

void Editor::add_content_browser()
{
    auto content_browser_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Content);
    m_editor_windows.emplace_back(content_browser_window);
}

void Editor::add_game()
{
    auto game_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Game);
    m_editor_windows.emplace_back(game_window);
}

void Editor::add_inspector()
{
    auto inspector_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Inspector);
    m_editor_windows.emplace_back(inspector_window);
}

void Editor::add_scene_hierarchy()
{
    auto hierarchy_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Hierarchy);
    m_editor_windows.emplace_back(hierarchy_window);
}

void Editor::remove_window(std::shared_ptr<EditorWindow> const& window)
{
    auto const it = std::ranges::find(m_editor_windows, window);

    if (it != m_editor_windows.end())
        m_editor_windows.erase(it);
}

}
