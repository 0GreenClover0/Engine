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

    void add_component_to_awake(std::shared_ptr<Component> const& component);
    void remove_component_to_awake(std::shared_ptr<Component> const& component);

    void add_component_to_start(std::shared_ptr<Component> const& component);
    void remove_component_to_start(std::shared_ptr<Component> const& component);

    void run_frame();

    bool is_running = false;

    std::vector<std::shared_ptr<Entity>> entities = {};
    std::vector<std::shared_ptr<Component>> tickable_components = {};

private:
    std::vector<std::shared_ptr<Component>> components_to_awake = {};
    std::vector<std::shared_ptr<Component>> components_to_start = {};

    friend class SceneSerializer;
};
