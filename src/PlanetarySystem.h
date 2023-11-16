#pragma once
#include <vector>

#include "AstronomicalObject.h"
#include "Component.h"
#include "Shader.h"

class PlanetarySystem final : public Component
{
public:
	explicit PlanetarySystem();
	virtual void awake() override;
	void change_detail(float detail) const;

	float x_rotation = 0.0f;
	float y_rotation = 0.0f;

private:
	static int constexpr planet_count = 10;
	static int constexpr moon_count = 7;
	std::vector<std::weak_ptr<AstronomicalObject>> planets = {};
	std::vector<std::weak_ptr<AstronomicalObject>> moons = {};

	int default_sphere_sector_and_stack_count = 72;
};
