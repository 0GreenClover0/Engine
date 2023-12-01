#include "Scene.h"

#include "Entity.h"

void Scene::add_child(std::shared_ptr<Entity> const& entity)
{
    entities.emplace_back(entity);
}

void Scene::awake()
{
    is_during_awake = true;

    // Scene Entities vector might be modified by components, ex. when they create new entities
    // TODO: Destroying entities is not handled properly. But we don't support any way of destroying an entity anyway, so...
    auto const entities_copy = entities;
    for (auto const& entity : entities_copy)
    {
        for (auto const& component : entity->components)
        {
            component->awake();
        }
    }

    is_during_awake = false;
    is_after_awake = true;
}

void Scene::start()
{
    is_during_start = true;

    // Scene Entities vector might be modified by components, ex. when they create new entities
    // TODO: Destroying entities is not handled properly. But we don't support any way of destroying an entity anyway, so...
    auto const entities_copy = entities;
    for (auto const& entity : entities_copy)
    {
        for (auto const& component : entity->components)
        {
            component->start();
        }
    }

    is_during_start = false;
    is_after_start = true;
}

void Scene::update() const
{
    // Scene Entities vector might be modified by components, ex. when they create new entities
    // TODO: Destroying entities is not handled properly. But we don't support any way of destroying an entity anyway, so...
    auto const entities_copy = entities;
    for (auto const& entity : entities_copy)
    {
        for (auto const& component : entity->components)
        {
            component->update();
        }

        for (auto const& drawable : entity->drawables)
        {
            // TODO: Group objects with the same shader together?
            drawable->material->shader->use();
            drawable->material->shader->set_mat4("model", entity->transform->get_model_matrix());
            drawable->draw();
        }
    }
}
