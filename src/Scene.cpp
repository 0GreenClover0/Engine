#include "Scene.h"

#include "AK/AK.h"
#include "Entity.h"
#include "ResourceManager.h"

void Scene::unload()
{
    // TODO: We should probably cache top level entities somewhere or maybe assign them to dummy root entity
    //       (I don't really like either of these).
    std::vector<std::shared_ptr<Entity>> top_level_entities = {};
    for (auto const& entity : entities)
    {
        if (entity->transform->parent.expired())
            top_level_entities.emplace_back(entity);
    }

    for (auto const& entity : top_level_entities)
    {
        entity->destroy_immediate();
    }

    ResourceManager::get_instance().reset_state();
}

void Scene::add_child(std::shared_ptr<Entity> const& entity)
{
    entities.emplace_back(entity);
}

void Scene::remove_child(std::shared_ptr<Entity> const& entity)
{
    auto const it = std::ranges::find(entities, entity);

    assert(it != entities.end());

    if (it == entities.end())
        return;

    entities.erase(it);
}

void Scene::add_component_to_awake(std::shared_ptr<Component> const& component)
{
    components_to_awake.emplace_back(component);
}

void Scene::remove_component_to_awake(std::shared_ptr<Component> const& component)
{
    // We don't want to change the order of the components Awakes
    if (auto const position = std::ranges::find(components_to_awake, component); position != components_to_awake.end())
    {
        components_to_awake.erase(position);
    }
}

void Scene::add_component_to_start(std::shared_ptr<Component> const& component)
{
    components_to_start.emplace_back(component);
}

void Scene::remove_component_to_start(std::shared_ptr<Component> const& component)
{
    // We don't want to change the order of the components Starts
    if (auto const position = std::ranges::find(components_to_start, component); position != components_to_start.end())
    {
        components_to_start.erase(position);
    }
}

std::shared_ptr<Component> Scene::get_component_by_guid(std::string const& guid) const
{
    // TODO: Cache components in an unordered map with guids as keys
    for (auto const& entity : entities)
    {
        for (auto const& component : entity->components)
        {
            if (component->guid == guid)
            {
                return component;
            }
        }
    }

    return nullptr;
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
            component->has_been_awaken = true;

            if (component->enabled())
                component->on_enabled();
        }

        components_to_awake.clear();

        // Release the capacity
        components_to_awake.shrink_to_fit();
    }

    // Call Start on every component that hasn't been started yet
    auto const copy_components_to_start = this->components_to_start;
    for (auto const& component : copy_components_to_start)
    {
        component->start();
        component->has_been_started = true;

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
        if (component == nullptr || component->entity == nullptr)
            continue;

        component->update();
    }
}
