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

    void awake();
    void start();
    void run_frame() const;

    std::vector<std::shared_ptr<Entity>> entities = {};
    std::vector<std::shared_ptr<Component>> tickable_components = {};

    bool is_during_awake = false;
    bool is_during_start = false;
    bool is_after_awake = false;
    bool is_after_start = false;

private:
    friend class SceneSerializer;
};
