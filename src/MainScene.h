#pragma once

#include "Scene.h"

class MainScene : public Scene
{
public:
	static void set_instance(std::shared_ptr<Scene> const& scene)
	{
		instance = scene;
	}

	static std::shared_ptr<Scene> get_instance()
	{
		return instance;
	}

	MainScene(MainScene const&) = delete;
	void operator=(MainScene const&) = delete;

private:
	inline static std::shared_ptr<Scene> instance;
};
