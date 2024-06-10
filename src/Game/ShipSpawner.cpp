#include "ShipSpawner.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/random.hpp>
#include <imgui.h>
#include <random>

#include "AK/AK.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Globals.h"
#include "Player.h"
#include "ResourceManager.h"
#include "ShipEyes.h"
#include <imgui_extensions.h>

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

        m_backup_spawn.emplace_back(s1);
        m_backup_spawn.emplace_back(s2);
        m_backup_spawn.emplace_back(s3);
        m_backup_spawn.emplace_back(s4);
        m_backup_spawn.emplace_back(s5);
    }
    else
    {
        m_backup_spawn.insert(m_backup_spawn.end(), m_main_event_spawn.begin(), m_main_event_spawn.end());
        m_main_event_spawn.clear();
    }

    for (u32 i = 0; i < m_backup_spawn.size(); i++)
    {
        if (m_backup_spawn[i].spawn_list.size() == 0)
        {
            Debug::log("Removed empty event!", DebugType::Warning);
            m_backup_spawn.erase(m_backup_spawn.begin() + i);
            i--;
        }
    }

    m_main_spawn = m_backup_spawn;

    auto const seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::ranges::shuffle(m_main_spawn, std::default_random_engine(seed));

    for (auto const& path : entity->get_components<Path>())
    {
        paths.emplace_back(path);
    }

    set_can_tick(true);
}

void ShipSpawner::update()
{
    prepare_for_spawn();
}

void ShipSpawner::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Light", light);
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

            m_main_event_spawn.emplace_back(new_element);
        }

        for (u32 i = 0; i < m_main_event_spawn.size(); i++)
        {
            ImGui::PushID(i);

            ImVec4 bg_color = {};
            ImVec4 text_color = {};

            ship_type_to_color(m_main_event_spawn[i].spawn_list[0], bg_color, text_color);

            ImGui::PushStyleColor(ImGuiCol_Text, text_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_color);
            ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_color);

            ImGui::SetNextItemWidth(half_width);
            if (ImGui::BeginCombo(("##Ship Type MainEvent" + std::to_string(i)).c_str(),
                                  ship_type_to_string(m_main_event_spawn[i].spawn_list[0]).c_str()))
            {
                for (u32 k = static_cast<u32>(ShipType::FoodSmall); k <= static_cast<u32>(ShipType::Tool); k++)
                {
                    bool const is_selected = m_main_event_spawn[i].spawn_list[0] == static_cast<ShipType>(k);

                    ship_type_to_color(static_cast<ShipType>(k), bg_color, text_color);

                    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                    ImGui::PushStyleColor(ImGuiCol_Header, bg_color);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(bg_color.x + 0.2f, bg_color.y + 0.2f, bg_color.z + 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, bg_color);

                    if (ImGui::Selectable(ship_type_to_string(static_cast<ShipType>(k)).c_str(), true))
                    {
                        m_main_event_spawn[i].spawn_list[0] = static_cast<ShipType>(k);
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
                m_main_event_spawn.erase(m_main_event_spawn.begin() + i);
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
            m_backup_spawn.emplace_back(new_element);
        }

        ImGui::Spacing();

        for (u32 i = 0; i < m_backup_spawn.size(); i++)
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
                m_backup_spawn[i].spawn_list.emplace_back(ShipType::FoodSmall);
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(third_width);

            if (ImGui::BeginCombo(("##SpawnType" + std::to_string(i)).c_str(), spawn_type_to_string(m_backup_spawn[i].spawn_type).c_str()))
            {
                for (u32 j = static_cast<u32>(SpawnType::Sequence); j <= static_cast<u32>(SpawnType::Rapid); j++)
                {
                    bool const is_selected = m_backup_spawn[i].spawn_type == static_cast<SpawnType>(j);

                    if (ImGui::Selectable(spawn_type_to_string(static_cast<SpawnType>(j)).c_str(), is_selected))
                    {
                        m_backup_spawn[i].spawn_type = static_cast<SpawnType>(j);
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
                m_backup_spawn.erase(m_backup_spawn.begin() + i);
                i = i - 1;
                continue;
            }

            if (m_backup_spawn[i].spawn_list.size() == 0)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 250, 0, 255));

                std::string const empty_event_warning = "Empty events will be removed!";
                text_size = ImGui::CalcTextSize(empty_event_warning.c_str());

                offset_x = (available.x - text_size.x) * 0.5f;
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

                ImGui::Text("%s", empty_event_warning.c_str());
                ImGui::PopStyleColor();
            }

            for (u32 j = 0; j < m_backup_spawn[i].spawn_list.size(); j++)
            {
                ImGui::PushID(j);

                ImGui::SetNextItemWidth(half_width);

                ImVec4 bg_color, text_color;
                ship_type_to_color(m_backup_spawn[i].spawn_list[j], bg_color, text_color);

                ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, bg_color);
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, bg_color);

                if (ImGui::BeginCombo(("##ShipType" + std::to_string(i) + std::to_string(j)).c_str(),
                                      ship_type_to_string(m_backup_spawn[i].spawn_list[j]).c_str()))
                {
                    for (u32 k = static_cast<u32>(ShipType::FoodSmall); k <= static_cast<u32>(ShipType::Tool); k++)
                    {
                        bool const is_selected = (m_backup_spawn[i].spawn_list[j] == static_cast<ShipType>(k));

                        ship_type_to_color(static_cast<ShipType>(k), bg_color, text_color);

                        ImGui::PushStyleColor(ImGuiCol_Text, text_color);
                        ImGui::PushStyleColor(ImGuiCol_Header, bg_color);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                                              ImVec4(bg_color.x + 0.2f, bg_color.y + 0.2f, bg_color.z + 0.2f, 1.0f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, bg_color);

                        if (ImGui::Selectable(ship_type_to_string(static_cast<ShipType>(k)).c_str(), true))
                        {
                            m_backup_spawn[i].spawn_list[j] = static_cast<ShipType>(k);
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
                    m_backup_spawn[i].spawn_list.erase(m_backup_spawn[i].spawn_list.begin() + j);
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

void ShipSpawner::add_warning()
{
    auto const warning = Entity::create("Warning");
    auto const warning_light_component = warning->add_component(SpotLight::create());

    warning_light_component->diffuse = glm::vec3(0.04f, 0.0f, 0.0f);
    warning_light_component->ambient = glm::vec3(0.0f);
    warning_light_component->specular = glm::vec3(1.0f);

    warning_light_component->cut_off = cos(glm::radians(29.0f));
    warning_light_component->outer_cut_off = cos(glm::radians(30.0f));

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

    if (Player::get_instance()->food < LevelController::get_instance()->map_food - last_chance_food_threshold)
    {
        return false;
    }

    m_backup_spawn.clear();
    m_main_spawn.clear();

    SpawnEvent last_chance_event = {};

    u32 food_shortage = LevelController::get_instance()->map_food - Player::get_instance()->food;

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
    if (!m_is_last_chance_activated && m_backup_spawn.empty())
    {
        Debug::log("Ship spawn list is empty!", DebugType::Error);
        return;
    }

    if (m_main_spawn.back().spawn_list.empty())
    {
        if (m_spawn_type == SpawnType::Immediate || m_spawn_type == SpawnType::Rapid)
        {
            m_spawn_warning_counter = spawn_warning_time;
        }
        m_main_spawn.pop_back();
    }

    if (m_main_spawn.empty())
    {
        m_main_spawn = m_backup_spawn;
        return;
    }

    if (paths.size() == 0)
    {
        Debug::log("No available paths to create ships on!", DebugType::Warning);
        return;
    }

    if (!m_is_last_chance_activated && is_time_for_last_chance())
    {
        return;
    }

    SpawnEvent* being_spawn = &m_main_spawn.back();
    m_spawn_type = being_spawn->spawn_type;

    if (m_spawn_warning_counter > 0.0f)
    {
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

            being_spawn->spawn_list.pop_back();

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

        m_is_half_rapid_done = false;

        m_spawn_warning_counter = spawn_warning_time;
    }
}

void ShipSpawner::spawn_ship(SpawnEvent const* being_spawn)
{
    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    auto const ship = Entity::create("Ship");
    ship->transform->set_local_position({m_spawn_position.back().x, 0.0f, m_spawn_position.back().y});

    auto const eyes = Entity::create("Eyes");
    eyes->transform->set_parent(ship->transform);
    auto const eyes_comp = eyes->add_component<ShipEyes>(ShipEyes::create());

    auto const collider_in_front = eyes->add_component<Collider2D>(Collider2D::create(0.1f, 0.2f));

    auto const ship_comp =
        ship->add_component(Ship::create(light.lock(), std::static_pointer_cast<ShipSpawner>(shared_from_this()), eyes_comp));
    auto const collider = ship->add_component<Collider2D>(Collider2D::create(1.0f, 1.0f));

    ship_comp->on_ship_destroyed.attach(&ShipSpawner::remove_ship, shared_from_this());
    ship_comp->maximum_speed = LevelController::get_instance()->ships_speed;

    m_ships.emplace_back(ship_comp);

    ship_comp->type = being_spawn->spawn_list.back();

    if (ship_comp->type == ShipType::FoodSmall)
    {
        ship->add_component(Model::create("./res/models/shipSmall/shipSmall.gltf", standard_material));
        collider->set_bounds_dimensions_2d(0.25f / 2.0f, 0.65f / 2.0f);
        collider->offset = {0.0f, 0.035f};
        collider_in_front->offset = {0.0f, -0.5f};
    }
    else if (ship_comp->type == ShipType::FoodMedium)
    {
        ship->add_component(Model::create("./res/models/shipMedium/shipMedium.gltf", standard_material));
        collider->set_bounds_dimensions_2d(0.5f / 2.0f, 1.1f / 2.0f);
        collider_in_front->set_bounds_dimensions_2d(0.2f / 2.0f, 0.5f / 2.0f);
        collider_in_front->offset = {0.0f, -0.8f};
    }
    else if (ship_comp->type == ShipType::FoodBig)
    {
        ship->add_component(Model::create("./res/models/shipBig/shipBig.gltf", standard_material));
        collider->set_bounds_dimensions_2d(0.5f / 2.0f, 1.6f / 2.0f);
        collider_in_front->set_bounds_dimensions_2d(0.2f / 2.0f, 0.7f / 2.0f);
        collider->offset = {0.0f, 0.005f};
        collider_in_front->offset = {0.0f, -1.15f};
    }
    else if (ship_comp->type == ShipType::Pirates)
    {
        ship->add_component(Model::create("./res/models/shipPirates/shipPirates.gltf", standard_material));
        collider->set_bounds_dimensions_2d(0.25f / 2.0f, 0.65f / 2.0f);
        collider->offset = {0.0f, 0.035f};
        collider_in_front->offset = {0.0f, -0.5f};
    }
    else if (ship_comp->type == ShipType::Tool)
    {
        ship->add_component(Model::create("./res/models/shipTool/shipTool.gltf", standard_material));
        collider->set_bounds_dimensions_2d(0.25f / 2.0f, 0.65f / 2.0f);
        collider->offset = {0.0f, 0.035f};
        collider_in_front->offset = {0.0f, -0.5f};
    }

    collider->is_trigger = true;
    collider_in_front->is_trigger = true;
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
