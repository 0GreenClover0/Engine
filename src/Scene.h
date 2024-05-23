#pragma once
#include <memory>
#include <vector>

#include "Component.h"

class Entity;

class Scene
{
public:
    Scene() = default;

    virtual void unload();

    void add_child(std::shared_ptr<Entity> const& entity);
    void remove_child(std::shared_ptr<Entity> const& entity);

    void add_component_to_awake(std::shared_ptr<Component> const& component);
    void remove_component_to_awake(std::shared_ptr<Component> const& component);

    void add_component_to_start(std::shared_ptr<Component> const& component);
    void remove_component_to_start(std::shared_ptr<Component> const& component);

    [[nodiscard]] std::shared_ptr<Entity> get_entity_by_guid(std::string const& guid) const;
    [[nodiscard]] std::shared_ptr<Component> get_component_by_guid(std::string const& guid) const;

    void run_frame();

    bool is_running = false;

    std::vector<std::shared_ptr<Entity>> entities = {};
    std::vector<std::shared_ptr<Component>> tickable_components = {};

private:
    std::vector<std::shared_ptr<Component>> components_to_awake = {};
    std::vector<std::shared_ptr<Component>> components_to_start = {};

    friend class SceneSerializer;
};
