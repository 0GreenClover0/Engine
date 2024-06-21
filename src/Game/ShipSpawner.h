#pragma once

#include "Component.h"
#include "FloatersManager.h"
#include "Path.h"
#include "Ship.h"

enum class SpawnType
{
    Sequence,
    Immediate,
    Rapid
};

struct SpawnEvent
{
    std::vector<ShipType> spawn_list = {};
    SpawnType spawn_type = SpawnType::Sequence;
};

class ShipSpawner final : public Component
{

public:
    static std::shared_ptr<ShipSpawner> create();
    static std::shared_ptr<ShipSpawner> create(std::shared_ptr<LighthouseLight> const& light);

    explicit ShipSpawner(AK::Badge<ShipSpawner>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    std::optional<glm::vec2> find_nearest_non_pirate_ship(std::shared_ptr<Ship> const& center_ship) const;
    std::optional<glm::vec2> find_nearest_ship_position(glm::vec2 center_position) const;
    std::optional<std::weak_ptr<Ship>> find_nearest_ship_object(glm::vec2 center_position) const;

    void get_spawn_paths();
    bool should_decal_be_drawn() const;

    std::vector<std::weak_ptr<Path>> paths = {};

    std::weak_ptr<FloatersManager> floaters_manager = {};
    std::weak_ptr<LighthouseLight> light = {};

    float const spawn_warning_time = 1.5f;
    float const spawn_rapid_time = 2.5f;
    float const minimum_spawn_distance = 2.25f;

    u32 last_chance_food_threshold = 5;
    float last_chance_time_threshold = 30.0f;

    std::vector<SpawnEvent> main_event_spawn = {};
    std::vector<SpawnEvent> backup_spawn = {};

    void burn_out_all_ships(bool const value) const;
    void spawn_ship_at_position(ShipType const type, glm::vec2 position, float const direction);
    void pop_event();
    void reset_event();

private:
    void spawn_ship(SpawnEvent const* being_spawn);
    void prepare_for_spawn();
    void remove_ship(std::shared_ptr<Ship> const& ship_to_remove);
    bool is_spawn_possible() const;
    bool is_time_for_last_chance();
    void add_warning();

    static std::string spawn_type_to_string(SpawnType const type);

    std::vector<SpawnEvent> m_main_spawn = {};

    std::vector<std::weak_ptr<Entity>> m_warning_lights = {};
    float m_spawn_warning_counter = 0.0f;
    std::vector<glm::vec2> m_spawn_position = {};

    std::vector<std::weak_ptr<Ship>> m_ships = {};

    bool m_is_test_spawn_enable = false;
    bool m_is_half_rapid_done = false;

    bool m_is_last_chance_activated = false;

    SpawnType m_spawn_type = SpawnType::Sequence;
};
