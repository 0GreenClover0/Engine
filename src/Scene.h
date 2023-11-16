 #pragma once
#include <memory>
#include <vector>

class Entity;

class Scene
{
public:
	Scene() = default;
	void add_child(std::shared_ptr<Entity> const& entity);

	void awake();
	void start();
	void update() const;

	std::vector<std::shared_ptr<Entity>> entities = {};

	bool is_during_awake = false;
	bool is_during_start = false;
	bool is_after_awake = false;
	bool is_after_start = false;
};
