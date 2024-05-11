#include "Port.h"

#include "Collider2D.h"
#include "Cube.h"
#include "Entity.h"
#include "Player.h"
#include "ResourceManager.h"
#include "Ship.h"
#include "AK/AK.h"
#include "AK/Badge.h"

std::shared_ptr<Port> Port::create()
{
    return std::make_shared<Port>(AK::Badge<Port> {});
}

Port::Port(AK::Badge<Port>)
{
}

void Port::on_trigger_enter(std::shared_ptr<Collider2D> const &other)
{
    auto const ship = other->entity->get_component<Ship>();
    if (ship != nullptr && !ship->is_in_port() && ship->type!=ShipType::Pirates)
    {
        ship->stop();
        ships_inside.emplace_back(ship);
    }
}

bool Port::interact()
{
    if (ships_inside.size() <= 0)
        return false;

    std::shared_ptr<Ship> chosen_ship = nullptr;

    for (auto const& ship : ships_inside)
    {
        if (ship.expired())
        {
            Debug::log("Trying to interact with an expired ship inside the port. Something went wrong with removing the ship from the vector?", DebugType::Error);
            continue;
        }

        if (ship.lock()->type != ShipType::Pirates)
        {
            chosen_ship = ship.lock();
            break;
        }
    }

    if (chosen_ship == nullptr)
    {
        return false;
    }

    auto const& ship = chosen_ship;

    switch (ship->type)
    {
    case ShipType::FoodSmall:
        Player::get_instance()->food += 1;
        break;

    case ShipType::FoodMedium:
        Player::get_instance()->food += 2;
        break;

    case ShipType::FoodBig:
        Player::get_instance()->food += 5;
        break;

    case ShipType::Tool:
        Player::get_instance()->packages += 1;
        break;

    case ShipType::Pirates:
        std::unreachable();

    default:
        break;
    }

    AK::erase(ships_inside, ship);
    ship->entity->destroy_immediate();
    return true;
}

float Port::get_interactable_distance() const
{
    return m_interactable_distance;
}
