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

void Port::on_collision_enter(std::shared_ptr<Collider2D> const &other)
{
    auto const ship = other->entity->get_component<Ship>();
    if (ship != nullptr && !ship->is_in_port())
    {
        ship->stop();
        ships_inside.emplace_back(ship);
    }
}

bool Port::interact()
{
    if (ships_inside.size() <= 0)
        return false;

    if (ships_inside[0].expired())
    {
        Debug::log("Trying to interact with an expired ship inside the port. Something went wrong with removing the ship from the vector?", DebugType::Error);
        return false;
    }

    auto const ship = ships_inside[0].lock();

    // TODO: Do different things based on ship type
    Player::get_instance()->packages += 1;
    AK::erase(ships_inside, ship);
    ship->entity->destroy_immediate();
    return true;
}

float Port::get_interactable_distance() const
{
    return m_interactable_distance;
}
