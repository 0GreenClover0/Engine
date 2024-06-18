#include "ShipEyes.h"

#include "AK/AK.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Globals.h"
#include "IceBound.h"
#include "Input.h"
#include "Ship.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#if EDITOR
#include <imgui.h>
#endif

std::shared_ptr<ShipEyes> ShipEyes::create()
{
    return std::make_shared<ShipEyes>(AK::Badge<ShipEyes> {});
}

ShipEyes::ShipEyes(AK::Badge<ShipEyes>)
{
}

void ShipEyes::awake()
{
    set_can_tick(true);
}

void ShipEyes::update()
{
}

void ShipEyes::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    if (other->entity->get_component<IceBound>() != nullptr)
    {
        see_obstacle = true;
    }
}

void ShipEyes::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{
    if (other->entity->get_component<IceBound>() != nullptr)
    {
        see_obstacle = false;
    }
}
