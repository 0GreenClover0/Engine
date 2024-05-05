#include "ShipSpawner.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <random>
#include <glm/gtc/random.hpp>

#include "Entity.h"
#include "Globals.h"
#include "ResourceManager.h"
#include "AK/AK.h"

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
    s2.spawn_list.emplace_back(ShipType::FoodMedium);
    s2.spawn_type = SpawnType::Sequence;

    SpawnEvent s3 = {};
    s3.spawn_list.emplace_back(ShipType::FoodBig);
    s3.spawn_type = SpawnType::Sequence;

    SpawnEvent pirate_event = {};
    pirate_event.spawn_list.emplace_back(ShipType::Pirates);
    pirate_event.spawn_list.emplace_back(ShipType::Pirates);
    pirate_event.spawn_list.emplace_back(ShipType::Pirates);
    pirate_event.spawn_list.emplace_back(ShipType::Pirates);
    pirate_event.spawn_type = SpawnType::Imidiet;

    m_backup_spawn.emplace_back(s1);
    m_backup_spawn.emplace_back(s2);
    m_backup_spawn.emplace_back(s3);
    m_backup_spawn.emplace_back(pirate_event);

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
    if (Input::input->get_key_down(GLFW_KEY_F2))
    {
        spawn_ship();
    }
}

void ShipSpawner::draw_editor()
{
}

void ShipSpawner::spawn_ship()
{       
    if (paths.size() == 0)
    {
        Debug::log("No available paths to create ships on!", DebugType::Warning);
        return;
    }

    std::weak_ptr<Path> const path = paths[std::rand() % paths.size()];

    glm::vec2 spawn_position = path.lock()->get_point_at(glm::linearRand(0.0f, 1.0f));

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    auto const ship = Entity::create("ship");
    ship->add_component(Model::create("./res/models/ship/ship.gltf", standard_material));
    ship->transform->set_local_position({ spawn_position.x, 0.0f, spawn_position.y });

    auto const ship_comp = ship->add_component(Ship::create(light.lock()));
    ship_comp->on_ship_destroyed.attach(&ShipSpawner::remove_ship, shared_from_this());

    ships.emplace_back(ship_comp);

    if (m_main_spawn.back().spawn_list.empty())
    {
        m_main_spawn.pop_back();
    }

    if (m_main_spawn.empty())
    {
        m_main_spawn = m_backup_spawn;
    }

    SpawnEvent* being_spawn = &m_main_spawn.back();

    being_spawn->spawn_list.pop_back();
}

void ShipSpawner::remove_ship(std::shared_ptr<Ship> const& ship_to_remove)
{
    for (i32 i = 0; i < ships.size(); i++)
    {
        if (ships[i].lock() == ship_to_remove)
        {
            ships.erase(ships.begin() + i);
        }
    }
}
