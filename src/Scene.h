#pragma once
#include <memory>
#include <vector>

#include "Component.h"

class Entity;

class Scene
{
public:
    Scene() = default;
    void add_child(std::shared_ptr<Entity> const& entity);

    void run_frame();

    bool is_running = false;

    std::vector<std::shared_ptr<Entity>> entities = {};
    std::vector<std::shared_ptr<Component>> tickable_components = {};
    std::vector<std::shared_ptr<Component>> components_to_start = {};
    std::vector<std::shared_ptr<Component>> components_to_awake = {};

private:
    friend class SceneSerializer;
};
