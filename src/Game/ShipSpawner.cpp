#include "ShipSpawner.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Floater.h"
#include "GameController.h"
#include "Globals.h"
#include "Player.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"
#include "ShipEyes.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/random.hpp>

#include <random>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<ShipSpawner> ShipSpawner::create()
{
    return std::make_shared<ShipSpawner>(AK::Badge<ShipSpawner> {});
}

std::shared_ptr<ShipSpawner> ShipSpawner::create(std::shared_ptr<LighthouseLight> const& light)
{
    auto ship_spawner = std::make_shared<ShipSpawner>(AK::Badge<ShipSpawner> {});
    ship_spawner->light = light;

    return ship_spawner;
}

ShipSpawner::ShipSpawner(AK::Badge<ShipSpawner>)
{
}

void ShipSpawner::awake()
{
    if (m_is_test_spawn_enable)
    {
        SpawnEvent s1 = {};
        s1.spawn_list.emplace_back(ShipType::FoodSmall);
        s1.spawn_type = SpawnType::Sequence;

        SpawnEvent s2 = {};
        s2.spawn_list.emplace_back(ShipType::FoodMedium);
        s2.spawn_type = SpawnType::Sequence;

        SpawnEvent s3 = {};
        s3.spawn_list.emplace_back(ShipType::FoodBig);
        s3.spawn_type = SpawnType::Sequence;

        SpawnEvent s4 = {};
        s4.spawn_list.emplace_back(ShipType::Pirates);
        s4.spawn_type = SpawnType::Sequence;

        SpawnEvent s5 = {};
        s5.spawn_list.emplace_back(ShipType::Tool);
        s5.spawn_type = SpawnType::Sequence;

        backup_spawn.emplace_back(s1);
        backup_spawn.emplace_back(s2);
        backup_spawn.emplace_back(s3);
        backup_spawn.emplace_back(s4);
        backup_spawn.emplace_back(s5);
    }
    else
    {
        backup_spawn.insert(backup_spawn.end(), main_event_spawn.begin(), main_event_spawn.end());
        main_event_spawn.clear();
    }

    for (u32 i = 0; i < backup_spawn.size(); i++)
    {
        if (backup_spawn[i].spawn_list.size() == 0)
        {
            Debug::log("Removed empty event!", DebugType::Warning);
            backup_spawn.erase(backup_spawn.begin() + i);
            i--;
        }
    }

    m_main_spawn = backup_spawn;

    if (LevelController::get_instance()->is_started)
    {
        auto const seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::ranges::shuffle(m_main_spawn, std::default_random_engine(seed));
    }

    get_spawn_paths();

    set_can_tick(true);
}

void ShipSpawner::get_spawn_paths()
{
    for (auto const& path : entity->get_components<Path>())
    {
        paths.emplace_back(path);
    }
}

bool ShipSpawner::should_decal_be_drawn() const
{
    if (light.lock()->enabled())
    {
        for (auto const& ship : m_ships)
        {
            if (ship.lock()->behavioral_state == BehavioralState::Control)
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

void ShipSpawner::update()
{
    if (LevelController::get_instance()->is_started)
    {
        prepare_for_spawn();
    }
}

#if EDITOR
void ShipSpawner::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Light", light);
    ImGuiEx::draw_ptr("Floater manager", floaters_manager);
    ImGui::Separator();

    ImGui::InputFloat("Last Chance time threshold", &last_chance_time_threshold);
    ImGui::InputScalar("Last Chance food threshold", ImGuiDataType_U32, &last_chance_food_threshold);

    ImGui::Separator();

    u32 index = 0;
    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();
        ImGui::Text(("Ship " + std::to_string(index)).c_str());
        ImGui::SameLine();
        std::string type_string = ship_type_to_string(ship_locked->type);
        std::string state_string = behaviour_state_to_string(ship_locked->behavioral_state);

        ImGui::Text(("Type " + type_string).c_str());
        ImGui::SameLine();
        ImGui::Text(("State " + state_string).c_str());
        if (!ship_locked->is_destroyed)
        {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
            if (ImGui::Button(("Destroy##" + std::to_string(index)).c_str()))
            {
                ship_locked->destroy();
            }
            ImGui::PopStyleColor(2);
        }

        index++;
    }

    ImGui::Separator();

    ImGui::Text(("Next ship counter " + std::to_string(m_spawn_warning_counter)).c_str());
    ImGui::Text(("Type " + spawn_type_to_string(m_spawn_type)).c_str());

    ImGui::Checkbox("Test spawn events", &m_is_test_spawn_enable);

    if (!m_is_test_spawn_enable)
    {
        ImGui::Separator();

        ImVec2 const available = ImGui::GetContentRegionAvail();

        float const half_width = available.x * 0.5f - ImGui::GetStyle().ItemSpacing.x;
        float const third_width = available.x * 0.33f - ImGui::GetStyle().ItemSpacing.x;

        std::string const main_event_header = "MAIN EVENT";
        ImVec2 text_size = ImGui::CalcTextSize(main_event_header.c_str());

        float offset_x = (available.x - text_size.x) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

        ImGui::Text("%s", main_event_header.c_str());

        if (ImGui::Button("Add Ship##MainEvent", ImVec2(-FLT_MIN, 20.0f)))
        {
            SpawnEvent new_element = {};
            new_element.spawn_list.emplace_back(ShipType::FoodSmall);
            new_element.spawn_type = SpawnType::Sequence;

            main_event_spawn.emplace_back(new_element);
        }

        for (u32 i = 0; i < main_event_spawn.size(); i++)
        {
            ImGui::PushID(i);

            ImVec4 bg_color = {};
            ImVec4 text_color = {};

            ship_type_to_color(main_event_spawn[i].spawn_list[0], bg_color, text_color);

            ImGui::PushStyleColor(ImGuiCol_Text, text_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_color);

            ImGui::SetNextItemWidth(half_width);
            if (ImGui::BeginCombo(("##Ship Type MainEvent" + std::to_string(i)).c_str(),
                                  ship_type_to_string(main_event_spawn[i].spawn_list[0]).c_str()))
            {
                for (u32 k = static_cast<u32>(ShipType::FoodSmall); k <= static_cast<u32>(ShipType::Tool); k++)
                {
                    bool const is_selected = main_event_spawn[i].spawn_list[0] == static_cast<ShipType>(k);

                    ship_type_to_color(static_cast<ShipType>(k), bg_color, text_color);

                    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                    ImGui::PushStyleColor(ImGuiCol_Header, bg_color);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(bg_color.x + 0.2f, bg_color.y + 0.2f, bg_color.z + 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, bg_color);

                    if (ImGui::Selectable(ship_type_to_string(static_cast<ShipType>(k)).c_str(), true))
                    {
                        main_event_spawn[i].spawn_list[0] = static_cast<ShipType>(k);
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }

                    ImGui::PopStyleColor(4);
                }
                ImGui::EndCombo();
            }

            ImGui::PopStyleColor(4);

            ImGui::SameLine();
            if (ImGui::Button(("Remove ship##" + std::to_string(i)).c_str(), ImVec2(half_width, 20)))
            {
                main_event_spawn.erase(main_event_spawn.begin() + i);
                i = i - 1;
                ImGui::PopID();
                continue;
            }

            ImGui::PopID();
        }

        ImGui::Spacing();

        std::string const custom_events_header = "CUSTOM EVENTS";
        text_size = ImGui::CalcTextSize(custom_events_header.c_str());

        offset_x = (available.x - text_size.x) * 0.5f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

        ImGui::Text("%s", custom_events_header.c_str());

        if (ImGui::Button("Add Event", ImVec2(-FLT_MIN, 20)))
        {
            SpawnEvent new_element = {};
            backup_spawn.emplace_back(new_element);
        }

        ImGui::Spacing();

        for (u32 i = 0; i < backup_spawn.size(); i++)
        {
            ImGui::PushStyleColor(ImGuiCol_Separator, IM_COL32(110, 110, 110, 255));
            ImGui::Separator();
            ImGui::PopStyleColor();

            std::string const event_header = "EVENT";
            text_size = ImGui::CalcTextSize(event_header.c_str());

            offset_x = (available.x - text_size.x) * 0.5f;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

            ImGui::Text("%s", event_header.c_str());

            if (ImGui::Button(("Add Ship##" + std::to_string(i)).c_str(), ImVec2(third_width, 20)))
            {
                backup_spawn[i].spawn_list.emplace_back(ShipType::FoodSmall);
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(third_width);

            if (ImGui::BeginCombo(("##SpawnType" + std::to_string(i)).c_str(), spawn_type_to_string(backup_spawn[i].spawn_type).c_str()))
            {
                for (u32 j = static_cast<u32>(SpawnType::Sequence); j <= static_cast<u32>(SpawnType::Rapid); j++)
                {
                    bool const is_selected = backup_spawn[i].spawn_type == static_cast<SpawnType>(j);

                    if (ImGui::Selectable(spawn_type_to_string(static_cast<SpawnType>(j)).c_str(), is_selected))
                    {
                        backup_spawn[i].spawn_type = static_cast<SpawnType>(j);
                    }

                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (ImGui::Button(("Remove Event##" + std::to_string(i)).c_str(), ImVec2(third_width, 20)))
            {
                backup_spawn.erase(backup_spawn.begin() + i);
                i = i - 1;
                continue;
            }

            if (backup_spawn[i].spawn_list.size() == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 250, 0, 255));

                std::string const empty_event_warning = "Empty events will be removed!";
                text_size = ImGui::CalcTextSize(empty_event_warning.c_str());

                offset_x = (available.x - text_size.x) * 0.5f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

                ImGui::Text("%s", empty_event_warning.c_str());
                ImGui::PopStyleColor();
            }

            for (u32 j = 0; j < backup_spawn[i].spawn_list.size(); j++)
            {
                ImGui::PushID(j);

                ImGui::SetNextItemWidth(half_width);

                ImVec4 bg_color, text_color;
                ship_type_to_color(backup_spawn[i].spawn_list[j], bg_color, text_color);

                ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_color);

                if (ImGui::BeginCombo(("##ShipType" + std::to_string(i) + std::to_string(j)).c_str(),
                                      ship_type_to_string(backup_spawn[i].spawn_list[j]).c_str()))
                {
                    for (u32 k = static_cast<u32>(ShipType::FoodSmall); k <= static_cast<u32>(ShipType::Tool); k++)
                    {
                        bool const is_selected = (backup_spawn[i].spawn_list[j] == static_cast<ShipType>(k));

                        ship_type_to_color(static_cast<ShipType>(k), bg_color, text_color);

                        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                        ImGui::PushStyleColor(ImGuiCol_Header, bg_color);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                              ImVec4(bg_color.x + 0.2f, bg_color.y + 0.2f, bg_color.z + 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, bg_color);

                        if (ImGui::Selectable(ship_type_to_string(static_cast<ShipType>(k)).c_str(), true))
                        {
                            backup_spawn[i].spawn_list[j] = static_cast<ShipType>(k);
                        }

                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }

                        ImGui::PopStyleColor(4);
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopStyleColor(4);

                ImGui::SameLine();
                if (ImGui::Button(("Remove ship##" + std::to_string(i) + std::to_string(j)).c_str(), ImVec2(half_width, 0.0f)))
                {
                    backup_spawn[i].spawn_list.erase(backup_spawn[i].spawn_list.begin() + j);
                    j = j - 1;
                    ImGui::PopID();
                    continue;
                }

                ImGui::PopID();
            }

            ImGui::Spacing();
        }
    }
}
#endif

void ShipSpawner::add_warning()
{
    auto const warning = Entity::create("Warning");
    warning->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);

    auto const warning_light_component = warning->add_component(PointLight::create());

    warning_light_component->diffuse = glm::vec3(0.7f, 0.0f, 0.0f);
    warning_light_component->ambient = glm::vec3(0.0f);
    warning_light_component->specular = glm::vec3(0.0f);

    //warning_light_component->linear = 10.0f;
    //warning_light_component->quadratic = 10.0f;

    warning->transform->set_local_position({m_spawn_position[0].x - glm::sign(m_spawn_position[0].x) * 1.5f, 1.0f, m_spawn_position[0].y});
    warning->transform->set_euler_angles({-90.0f, 0.0f, 0.0f});

    if (m_spawn_type == SpawnType::Rapid)
    {
        m_warning_lights.insert(m_warning_lights.begin(), warning);
    }
    else
    {
        m_warning_lights.emplace_back(warning);
    }
}

bool ShipSpawner::is_time_for_last_chance()
{
    if (LevelController::get_instance()->time > last_chance_time_threshold)
    {
        return false;
    }

    i32 food_shortage = LevelController::get_instance()->map_food - Player::get_instance()->food;

    if (food_shortage <= 0)
    {
        return false;
    }

    if (food_shortage > last_chance_food_threshold)
    {
        return false;
    }

    backup_spawn.clear();
    m_main_spawn.clear();

    SpawnEvent last_chance_event = {};

    while (food_shortage > 0)
    {
        Debug::log(std::to_string(food_shortage));

        if (food_shortage >= 5)
        {
            last_chance_event.spawn_list.emplace_back(ShipType::FoodBig);
            food_shortage -= 5;
            continue;
        }

        if (food_shortage >= 3)
        {
            last_chance_event.spawn_list.emplace_back(ShipType::FoodMedium);
            food_shortage -= 3;
            continue;
        }

        if (food_shortage >= 1)
        {
            last_chance_event.spawn_list.emplace_back(ShipType::FoodSmall);
            food_shortage -= 1;
            continue;
        }

        std::unreachable();
    }

    last_chance_event.spawn_type = SpawnType::Rapid;

    m_main_spawn.emplace_back(last_chance_event);

    m_is_last_chance_activated = true;
    Debug::log("LAST CHANCE!");

    return true;
}

void ShipSpawner::prepare_for_spawn()
{
    if (!m_is_last_chance_activated && backup_spawn.empty())
    {
        Debug::log("Ship spawn list is empty!", DebugType::Error);
        return;
    }

    if (m_main_spawn.empty())
    {
        if (!m_is_last_chance_activated)
        {
            m_main_spawn = backup_spawn;
        }
        return;
    }

    if (m_main_spawn.back().spawn_list.empty())
    {
        if (m_spawn_type == SpawnType::Immediate || m_spawn_type == SpawnType::Rapid)
        {
            m_spawn_warning_counter = spawn_warning_time;
        }
        m_main_spawn.pop_back();

        if (m_main_spawn.empty())
        {
            return;
        }
    }

    if (paths.size() == 0)
    {
        Debug::log("No available paths to create ships on!", DebugType::Warning);
        return;
    }

    if (!m_is_last_chance_activated)
    {
        bool const was_just_activated = is_time_for_last_chance();
        if (was_just_activated)
        {
            return;
        }
    }

    SpawnEvent* being_spawn = &m_main_spawn.back();
    m_spawn_type = being_spawn->spawn_type;

    if (m_spawn_warning_counter > 0.0f)
    {
        if (!AK::Math::are_nearly_equal(Player::get_instance()->flash_counter, 0.0f))
        {
            return;
        }

        if (m_spawn_type != SpawnType::Rapid)
        {
            m_spawn_warning_counter -= delta_time;
            return;
        }

        if (m_spawn_warning_counter > spawn_rapid_time || m_is_half_rapid_done)
        {
            m_spawn_warning_counter -= delta_time;
            return;
        }

        if (m_warning_lights.size() == 2)
        {
            if (m_ships.size() != 0)
            {
                auto const nearest_ship_position = find_nearest_ship_position(m_spawn_position.back());
                assert(nearest_ship_position.has_value());

                if (glm::distance(nearest_ship_position.value(), m_spawn_position.back()) < minimum_spawn_distance)
                {
                    m_spawn_warning_counter = spawn_warning_time;
                    return;
                }
            }

            m_warning_lights.back().lock()->destroy_immediate();
            m_warning_lights.pop_back();

            spawn_ship(being_spawn);

            m_spawn_position.pop_back();

            being_spawn->spawn_list.pop_back();

            if (being_spawn->spawn_list.size() > 1)
            {
                std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
                m_spawn_position.insert(m_spawn_position.begin(), path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f)));

                add_warning();
            }
        }
        else
        {
            if (being_spawn->spawn_list.size() > 1)
            {
                std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
                m_spawn_position.insert(m_spawn_position.begin(), path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f)));

                add_warning();
            }
            else
            {
                m_warning_lights.back().lock()->destroy_immediate();
                m_warning_lights.pop_back();

                spawn_ship(being_spawn);

                m_spawn_position.pop_back();

                being_spawn->spawn_list.pop_back();
            }
        }

        m_is_half_rapid_done = true;

        m_spawn_warning_counter -= delta_time;
        return;
    }

    if (m_spawn_type == SpawnType::Sequence)
    {
        if (!is_spawn_possible())
        {
            return;
        }

        if (m_warning_lights.size() != 0)
        {
            if (m_ships.size() != 0)
            {
                auto const nearest_ship_position = find_nearest_ship_position(m_spawn_position.back());
                assert(nearest_ship_position.has_value());

                if (glm::distance(nearest_ship_position.value(), m_spawn_position.back()) < minimum_spawn_distance)
                {
                    // There is no room near the spawning point, delay until next spawn time
                    m_spawn_warning_counter = spawn_warning_time;
                    return;
                }
            }

            m_warning_lights.back().lock()->destroy_immediate();
            m_warning_lights.pop_back();

            spawn_ship(being_spawn);

            m_spawn_position.pop_back();

            if (!LevelController::get_instance()->is_tutorial)
            {
                being_spawn->spawn_list.pop_back();
            }

            return;
        }

        std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
        m_spawn_position.emplace_back(path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f)));

        add_warning();

        m_spawn_warning_counter = spawn_warning_time;
    }

    if (m_spawn_type == SpawnType::Immediate)
    {
        if (m_warning_lights.size() != 0)
        {
            for (i32 i = m_warning_lights.size() - 1; i >= 0; i--)
            {
                m_warning_lights[i].lock()->destroy_immediate();
                m_warning_lights.pop_back();

                spawn_ship(being_spawn);

                m_spawn_position.pop_back();

                being_spawn->spawn_list.pop_back();
            }
        }
        else
        {
            for (auto const& spawn : being_spawn->spawn_list)
            {
                glm::vec2 potential_spawn_point = {};

                // NOTE: 100 is an arbitrary number of tries that we perform to find a suitable spawn point.
                //       If this number is reached we just don't spawn any more ships from this event.
                for (u32 i = 0; i < 100; i++)
                {
                    std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
                    potential_spawn_point = path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f));

                    auto nearest_ship_position = find_nearest_ship_position(potential_spawn_point);
                    if (!nearest_ship_position.has_value()
                        || glm::distance(nearest_ship_position.value(), potential_spawn_point) >= minimum_spawn_distance)
                    {
                        break;
                    }
                }

                m_spawn_position.emplace_back(potential_spawn_point);

                add_warning();
            }

            m_spawn_warning_counter = spawn_warning_time;
        }
    }

    if (m_spawn_type == SpawnType::Rapid)
    {
        if (m_warning_lights.size() == 2)
        {
            if (m_ships.size() != 0)
            {
                auto const nearest_ship_position = find_nearest_ship_position(m_spawn_position.back());
                assert(nearest_ship_position.has_value());

                if (glm::distance(nearest_ship_position.value(), m_spawn_position.back()) < minimum_spawn_distance)
                {
                    // There is no room near the spawning point, delay until next spawn time
                    m_spawn_warning_counter = spawn_warning_time;
                    return;
                }
            }

            if (m_warning_lights.size() == 0)
            {
                Debug::log("There is no warning but one should be destroyed!", DebugType::Error);
                return;
            }
            m_warning_lights.back().lock()->destroy_immediate();
            m_warning_lights.pop_back();

            spawn_ship(being_spawn);

            m_spawn_position.pop_back();

            being_spawn->spawn_list.pop_back();

            if (being_spawn->spawn_list.size() > 1)
            {
                std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
                m_spawn_position.insert(m_spawn_position.begin(), path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f)));

                add_warning();
            }
        }
        else
        {
            if (being_spawn->spawn_list.size() > 1)
            {
                std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];
                m_spawn_position.insert(m_spawn_position.begin(), path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f)));

                add_warning();
            }
            else
            {
                if (m_warning_lights.size() == 0)
                {
                    Debug::log("There is no warning but one should be destroyed!", DebugType::Error);
                    return;
                }

                m_warning_lights.back().lock()->destroy_immediate();
                m_warning_lights.pop_back();

                spawn_ship(being_spawn);

                m_spawn_position.pop_back();

                being_spawn->spawn_list.pop_back();
            }
        }

        m_is_half_rapid_done = false;

        m_spawn_warning_counter = spawn_warning_time;
    }
}

void ShipSpawner::burn_out_all_ships(bool const value) const
{
    if (value)
    {
        for (auto ship : m_ships)
        {
            ship.lock()->my_light.lock()->set_burn_out(true);
        }
    }
    else
    {
        for (auto ship : m_ships)
        {
            ship.lock()->my_light.lock()->set_enabled(true);
        }
    }
}

void ShipSpawner::spawn_ship(SpawnEvent const* being_spawn)
{
    std::shared_ptr<Entity> ship;

    FloaterSettings spawning_boat_settings = {};
    if (being_spawn->spawn_list.back() == ShipType::FoodSmall)
    {
        ship = SceneSerializer::load_prefab("ShipSmall");
        spawning_boat_settings = floaters_manager.lock()->small_boat_settings;
    }
    else if (being_spawn->spawn_list.back() == ShipType::FoodMedium)
    {
        ship = SceneSerializer::load_prefab("ShipMedium");
        spawning_boat_settings = floaters_manager.lock()->medium_boat_settings;
    }
    else if (being_spawn->spawn_list.back() == ShipType::FoodBig)
    {
        ship = SceneSerializer::load_prefab("ShipBig");
        spawning_boat_settings = floaters_manager.lock()->big_boat_settings;
    }
    else if (being_spawn->spawn_list.back() == ShipType::Pirates)
    {
        ship = SceneSerializer::load_prefab("ShipPirates");
        spawning_boat_settings = floaters_manager.lock()->pirate_boat_settings;
    }
    else if (being_spawn->spawn_list.back() == ShipType::Tool)
    {
        ship = SceneSerializer::load_prefab("ShipTool");
        spawning_boat_settings = floaters_manager.lock()->tool_boat_settings;
    }
    auto const floater = ship->add_component(
        Floater::create(floaters_manager.lock()->water, spawning_boat_settings.sink_rate, spawning_boat_settings.side_floaters_offset,
                        spawning_boat_settings.side_rotation_strength, spawning_boat_settings.forward_rotation_strength,
                        spawning_boat_settings.forward_floaters_offset));
    ship->transform->set_local_position({m_spawn_position.back().x, 0.0f, m_spawn_position.back().y});
    ship->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);

    auto const& ship_comp = ship->get_component<Ship>();
    ship_comp->on_ship_destroyed.attach(&ShipSpawner::remove_ship, shared_from_this());
    ship_comp->maximum_speed = LevelController::get_instance()->ships_speed;
    ship_comp->light = light;
    ship_comp->spawner = std::static_pointer_cast<ShipSpawner>(shared_from_this());
    ship_comp->floater = floater;

    ship_comp->set_start_direction();

    m_ships.emplace_back(ship->get_component<Ship>());
}

void ShipSpawner::spawn_ship_at_position(ShipType type, glm::vec2 position, float direction)
{
    std::shared_ptr<Entity> ship;

    FloaterSettings spawning_boat_settings = {};
    if (type == ShipType::FoodSmall)
    {
        ship = SceneSerializer::load_prefab("ShipSmall");
        spawning_boat_settings = floaters_manager.lock()->small_boat_settings;
    }
    else if (type == ShipType::FoodMedium)
    {
        ship = SceneSerializer::load_prefab("ShipMedium");
        spawning_boat_settings = floaters_manager.lock()->medium_boat_settings;
    }
    else if (type == ShipType::FoodBig)
    {
        ship = SceneSerializer::load_prefab("ShipBig");
        spawning_boat_settings = floaters_manager.lock()->big_boat_settings;
    }
    else if (type == ShipType::Pirates)
    {
        ship = SceneSerializer::load_prefab("ShipPirates");
        spawning_boat_settings = floaters_manager.lock()->pirate_boat_settings;
    }
    else if (type == ShipType::Tool)
    {
        ship = SceneSerializer::load_prefab("ShipTool");
        spawning_boat_settings = floaters_manager.lock()->tool_boat_settings;
    }
    auto const floater = ship->add_component(
        Floater::create(floaters_manager.lock()->water, spawning_boat_settings.sink_rate, spawning_boat_settings.side_floaters_offset,
                        spawning_boat_settings.side_rotation_strength, spawning_boat_settings.forward_rotation_strength,
                        spawning_boat_settings.forward_floaters_offset));
    ship->transform->set_local_position({position.x, 0.0f, position.y});
    ship->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);

    auto const& ship_comp = ship->get_component<Ship>();
    ship_comp->on_ship_destroyed.attach(&ShipSpawner::remove_ship, shared_from_this());
    ship_comp->maximum_speed = LevelController::get_instance()->ships_speed;
    ship_comp->light = light;
    ship_comp->spawner = std::static_pointer_cast<ShipSpawner>(shared_from_this());
    ship_comp->floater = floater;

    ship_comp->set_start_direction();
    ship_comp->behavioral_state = BehavioralState::Stop;
    ship_comp->set_direction(direction);

    m_ships.emplace_back(ship->get_component<Ship>());
}

bool ShipSpawner::is_spawn_possible() const
{
    i32 number_of_ships = 0;

    for (auto const& ship : m_ships)
    {
        if (!ship.lock()->is_destroyed)
        {
            number_of_ships++;
        }
    }

    if (number_of_ships < LevelController::get_instance()->ships_limit)
    {
        return true;
    }

    return false;
}

std::string ShipSpawner::spawn_type_to_string(SpawnType const type)
{
    switch (type)
    {
    case SpawnType::Sequence:
        return "Sequence";

    case SpawnType::Immediate:
        return "Immediate";

    case SpawnType::Rapid:
        return "Rapid";

    default:
        return "Undefined spawn";
    }
}

void ShipSpawner::remove_ship(std::shared_ptr<Ship> const& ship_to_remove)
{
    AK::swap_and_erase(m_ships, ship_to_remove);
}

std::optional<glm::vec2> ShipSpawner::find_nearest_non_pirate_ship(std::shared_ptr<Ship> const& center_ship) const
{
    bool found_non_pirate_ship = false;
    std::weak_ptr<Ship> nearest = {};
    for (auto const& ship : m_ships)
    {
        if (ship.lock()->type == ShipType::Pirates)
            continue;

        found_non_pirate_ship = true;
        nearest = ship;
        break;
    }

    if (!found_non_pirate_ship)
        return std::nullopt;

    glm::vec2 const ship_position = AK::convert_3d_to_2d(center_ship->entity->transform->get_local_position());
    glm::vec2 nearest_position = AK::convert_3d_to_2d(nearest.lock()->entity->transform->get_local_position());
    float nearest_distance = glm::distance(ship_position, nearest_position);

    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();
        if (ship_locked == center_ship || ship_locked->type == ShipType::Pirates || ship_locked->is_destroyed)
        {
            continue;
        }

        glm::vec2 position = AK::convert_3d_to_2d(ship_locked->entity->transform->get_local_position());
        float const distance = glm::distance(ship_position, position);

        if (nearest_distance > distance)
        {
            nearest_distance = distance;
            nearest_position = position;
        }
    }

    return nearest_position;
}

std::optional<glm::vec2> ShipSpawner::find_nearest_ship_position(glm::vec2 center_position) const
{
    if (m_ships.empty())
    {
        return std::nullopt;
    }

    auto const& nearest = m_ships[0];
    glm::vec2 nearest_position = AK::convert_3d_to_2d(nearest.lock()->entity->transform->get_local_position());
    float nearest_distance = glm::distance(center_position, nearest_position);

    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();

        glm::vec2 position = AK::convert_3d_to_2d(ship_locked->entity->transform->get_local_position());
        float const distance = glm::distance(center_position, position);

        if (nearest_distance > distance)
        {
            nearest_distance = distance;
            nearest_position = position;
        }
    }

    return nearest_position;
}

std::optional<std::weak_ptr<Ship>> ShipSpawner::find_nearest_ship_object(glm::vec2 center_position) const
{
    if (m_ships.empty())
    {
        return std::nullopt;
    }

    auto nearest = m_ships[0];
    auto nearest_locked = nearest.lock();
    if (!nearest_locked)
    {
        return std::nullopt;
    }

    glm::vec2 nearest_position = AK::convert_3d_to_2d(nearest_locked->entity->transform->get_local_position());
    float nearest_distance = glm::distance(center_position, nearest_position);

    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();
        if (!ship_locked)
        {
            continue;
        }

        glm::vec2 position = AK::convert_3d_to_2d(ship_locked->entity->transform->get_local_position());
        float const distance = glm::distance(center_position, position);

        if (nearest_distance > distance)
        {
            nearest_distance = distance;
            nearest = ship;
        }
    }

    return nearest;
}
