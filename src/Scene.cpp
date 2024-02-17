#include "Scene.h"

#include "AK.h"
#include "Entity.h"

void Scene::add_child(std::shared_ptr<Entity> const& entity)
{
    entities.emplace_back(entity);
}

void Scene::run_frame()
{
    // Call Awake on every component that was constructed before running the first frame
    if (!is_running)
    {
        is_running = true;

        auto const copy_components_to_awake = this->components_to_awake;
        for (auto const& component : components_to_awake)
        {
            component->awake();
        }

        components_to_awake.clear();

        // Release the capacity
        components_to_awake.shrink_to_fit();
    }

    // Call Start on every component that hasn't been started yet
    auto const copy_components_to_start = this->components_to_start;
    for (auto const& component : copy_components_to_start)
    {
        if (component != nullptr)
        {
            component->start();
            component->has_been_started = true;
        }

        AK::swap_and_erase(this->components_to_start, component);
    }

    // Call Update on every tickable component

    // Scene Entities vector might be modified by components, ex. when they create new entities
    // TODO: Destroying entities is not handled properly. But we don't support any way of destroying an entity anyway, so...

    // TODO: Don't make a copy of tickable components every frame, since they will most likely not change frequently, so we might
    //       just manually manage the vector?
    auto const components_copy = tickable_components;
    for (auto const& component : components_copy)
    {
        component->update();
    }
}
