#include "AstronomicalObject.h"

#include "Entity.h"
#include "Globals.h"
#include "Transform.h"

void AstronomicalObject::update()
{
	std::shared_ptr<Transform> const planet_transform = this->entity->transform;

    // Move in orbit
    planet_transform->set_local_position(glm::vec3(
        this->orbit.x * glm::cos(this->alpha),
        0.0f,
        this->orbit.y * glm::sin(this->alpha))
    );

    // Rotate around its own axis
    planet_transform->set_euler_angles(glm::vec3(
		planet_transform->get_euler_angles().x,
        planet_transform->get_euler_angles().y + delta_time * this->rotation_speed,
        planet_transform->get_euler_angles().z
    ));
      
    this->alpha += delta_time * this->speed;
}
