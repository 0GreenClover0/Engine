#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Component.h"
#include "Model.h"

class AstronomicalObject : public Component
{
public:
	float alpha;
	float speed;
	float rotation_speed;
	glm::vec2 orbit;

	std::weak_ptr<Model> model;

	void update() override;
};
