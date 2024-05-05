#include "Port.h"

#include "Collider2D.h"
#include "Cube.h"
#include "Entity.h"
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
