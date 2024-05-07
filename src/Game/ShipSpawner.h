#pragma once

#include "Component.h"
#include "Ship.h"
#include "Path.h"

class ShipSpawner final : public Component
{
public:
    static std::shared_ptr<ShipSpawner> create();
    static std::shared_ptr<ShipSpawner> create(std::shared_ptr<LighthouseLight> const& light);

    explicit ShipSpawner(AK::Badge<ShipSpawner>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    std::vector<std::weak_ptr<Path>> paths = {};

    std::weak_ptr<LighthouseLight> light = {};

private:
    void spawn_ship();
    void remove_ship(std::shared_ptr<Ship> const& ship_to_remove);
    bool is_spawn_possible() const;

    enum class ShipType
    {
        FoodSmall,
        FoodMedium,
        FoodBig,
        Pirates,
        Tool
    };

    enum class SpawnType
    {
        Sequence,
        Imidiet,
        Rapid
    };

    struct SpawnEvent
    {
        std::vector<ShipType> spawn_list = {};
        SpawnType spawn_type = SpawnType::Sequence;
    };

    std::vector<SpawnEvent> m_main_spawn = {};
    std::vector<SpawnEvent> m_backup_spawn = {};

    std::vector<std::weak_ptr<Ship>> ships = {};
};
