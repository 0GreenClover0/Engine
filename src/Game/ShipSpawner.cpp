#include "ShipSpawner.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <random>
#include <glm/gtc/random.hpp>

#include "Entity.h"
#include "Globals.h"
#include "ResourceManager.h"
#include "AK/AK.h"
#include "Collider2D.h"

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
    SpawnEvent s1 = {};
    s1.spawn_list.emplace_back(ShipType::FoodSmall);
    s1.spawn_type = SpawnType::Sequence;
    
    SpawnEvent s2 = {};
    s2.spawn_list.emplace_back(ShipType::FoodSmall);
    s2.spawn_type = SpawnType::Sequence;

    SpawnEvent s3 = {};
    s3.spawn_list.emplace_back(ShipType::FoodSmall);
    s3.spawn_type = SpawnType::Sequence;

    SpawnEvent s4 = {};
    s4.spawn_list.emplace_back(ShipType::Tool);
    s4.spawn_type = SpawnType::Sequence;

    m_backup_spawn.emplace_back(s1);
    m_backup_spawn.emplace_back(s2);
    m_backup_spawn.emplace_back(s3);
    m_backup_spawn.emplace_back(s4);

    m_main_spawn = m_backup_spawn;

    auto const seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::ranges::shuffle(m_main_spawn, std::default_random_engine(seed));

    auto const path1 = entity->add_component<Path>(Path::create());
    path1->add_points({ { -5.0f, -2.5f }, { -5.0f, 1.7f } });

    auto const path2 = entity->add_component<Path>(Path::create());
    path2->add_points({ { 5.0f, -2.5f }, { 5.0f, 1.7f } });

    for (auto const& path : entity->get_components<Path>())
    {
        paths.emplace_back(path);
    }

    set_can_tick(true);
}

void ShipSpawner::update()
{
    spawn_ship();
}

void ShipSpawner::draw_editor()
{
    u32 i = 0;
    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();
        ImGui::Text(("Ship " + std::to_string(i)).c_str());
        ImGui::SameLine();
        std::string type_string = ship_type_to_string(ship_locked->type);

        ImGui::Text(("Type " + type_string).c_str());
        if (!ship_locked->is_destroyed)
        {
            ImGui::SameLine();
            if (ImGui::Button(("Destroy##" + std::to_string(i)).c_str()))
            {
                ship_locked->destroy();
            }
        }

        i++;
    }
}

void ShipSpawner::spawn_ship()
{       
    if (paths.size() == 0)
    {
        Debug::log("No available paths to create ships on!", DebugType::Warning);
        return;
    }

    if (!is_spawn_possible())
    {
        return;
    }

    std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];

    glm::vec2 spawn_position = path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f));

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    auto const ship = Entity::create("ship");
    ship->add_component(Model::create("./res/models/ship/ship.gltf", standard_material));
    ship->transform->set_local_position({ spawn_position.x, 0.0f, spawn_position.y });

    auto const ship_comp = ship->add_component(Ship::create(light.lock(), std::static_pointer_cast<ShipSpawner>(shared_from_this())));
    auto const collider = ship->add_component<Collider2D>(Collider2D::create({ 0.1f, 0.1f }));
    collider->set_is_trigger(true);
    ship_comp->on_ship_destroyed.attach(&ShipSpawner::remove_ship, shared_from_this());

    m_ships.emplace_back(ship_comp);

    if (m_main_spawn.back().spawn_list.empty())
    {
        m_main_spawn.pop_back();
    }

    if (m_main_spawn.empty())
    {
        m_main_spawn = m_backup_spawn;
    }

    SpawnEvent* being_spawn = &m_main_spawn.back();

    ship_comp->type = being_spawn->spawn_list.back();

    being_spawn->spawn_list.pop_back();
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

void ShipSpawner::remove_ship(std::shared_ptr<Ship> const& ship_to_remove)
{
    AK::swap_and_erase(m_ships, ship_to_remove);
}

glm::vec2 ShipSpawner::find_nearest_non_pirate_ship(std::shared_ptr<Ship> const& center_ship)
{
    auto const& nearest = m_ships[0];
    glm::vec2 ship_position = AK::convert_3d_to_2d(center_ship->entity->transform->get_local_position());
    glm::vec2 nearest_position = AK::convert_3d_to_2d(nearest.lock()->entity->transform->get_local_position());
    float distance = glm::distance(ship_position, nearest_position);

    for (auto const& ship : m_ships)
    {
        auto const ship_locked = ship.lock();
        if (ship_locked == center_ship || ship_locked->type == ShipType::Pirates || ship_locked->is_destroyed)
        {
            continue;
        }

        glm::vec2 check_position = AK::convert_3d_to_2d(ship_locked->entity->transform->get_local_position());
        float check_distance = glm::distance(ship_position, check_position);

        if (distance < check_distance)
        {
            distance = check_distance;
            nearest_position = check_position;
        }
    }

    return nearest_position;
}
