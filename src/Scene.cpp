#include "Scene.h"

#include "Entity.h"

void Scene::add_child(std::shared_ptr<Entity> const& entity)
{
    entities.emplace_back(entity);
}

void Scene::awake()
{
    is_during_awake = true;

    for (auto const& entity : entities)
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

    for (auto const& entity : entities)
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
    for (auto const& child : entities)
    {
        for (auto const& component : child->components)
        {
            component->update();
        }

        for (auto const& drawable : child->drawables)
        {
            // TODO: Group objects with the same shader together?
            drawable->material->shader->use();
            drawable->material->shader->set_mat4("model", child->transform->get_model_matrix());
            drawable->draw();
        }
    }
}
