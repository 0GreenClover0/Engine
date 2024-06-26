#include "HovercraftWithoutKeeper.h"

#include "Collider2D.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"

#include <cmath>
#include <glm/common.hpp>
#include <glm/ext/quaternion_geometric.hpp>

std::shared_ptr<HovercraftWithoutKeeper> HovercraftWithoutKeeper::create()
{
    return std::make_shared<HovercraftWithoutKeeper>(AK::Badge<HovercraftWithoutKeeper> {});
}

HovercraftWithoutKeeper::HovercraftWithoutKeeper(AK::Badge<HovercraftWithoutKeeper>)
{
}

void HovercraftWithoutKeeper::awake()
{
    set_can_tick(true);

    m_collider = entity->get_component<Collider2D>();
}

void HovercraftWithoutKeeper::update()
{
    if (GameController::get_instance() != nullptr)
    {
        if (GameController::get_instance()->is_moving_to_next_scene())
        {
            m_collider.lock()->set_enabled(false);
        }
        else
        {
            m_collider.lock()->set_enabled(true);
        }
    }
    speed.x -= glm::sign(speed.x) * m_deceleration;
    speed.y -= glm::sign(speed.y) * m_deceleration;

    if (abs(speed.x) < m_deceleration)
    {
        speed.x = 0.0f;
    }

    if (abs(speed.y) < m_deceleration)
    {
        speed.y = 0.0f;
    }

    if (glm::length(speed) > m_maximum_speed)
    {
        speed = glm::normalize(speed) * m_maximum_speed;
    }

    glm::vec3 speed_vector = {speed.x, 0.0f, speed.y};
    speed_vector *= delta_time;

    entity->transform->set_local_position(entity->transform->get_local_position() + speed_vector);
}
