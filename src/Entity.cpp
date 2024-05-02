#include "Entity.h"

#include "Engine.h"
#include "MainScene.h"
#include "AK/AK.h"

Entity::Entity(AK::Badge<Entity>, std::string const& name) : name(std::move(name))
{
}

std::shared_ptr<Entity> Entity::create(std::string const& name)
{
    auto entity = std::make_shared<Entity>(AK::Badge<Entity> {}, name);
    entity->guid = AK::generate_guid();
    std::hash<std::string> constexpr hasher;
    entity->hashed_guid = hasher(entity->guid);
    entity->transform = std::make_shared<Transform>(entity);
    MainScene::get_instance()->add_child(entity);
    return entity;
}

std::shared_ptr<Entity> Entity::create(std::string const& guid, std::string const& name)
{
    auto entity = std::make_shared<Entity>(AK::Badge<Entity> {}, name);
    entity->guid = guid;
    std::hash<std::string> constexpr hasher;
    entity->hashed_guid = hasher(entity->guid);
    entity->transform = std::make_shared<Transform>(entity);
    MainScene::get_instance()->add_child(entity);
    return entity;
}

// Entity that is not tied to any scene
std::shared_ptr<Entity> Entity::create_internal(std::string const &name)
{
    auto entity = std::make_shared<Entity>(AK::Badge<Entity> {}, name);
    entity->guid = AK::generate_guid();
    std::hash<std::string> constexpr hasher;
    entity->hashed_guid = hasher(entity->guid);
    entity->transform = std::make_shared<Transform>(entity);
    return entity;
}

void Entity::destroy_immediate()
{
    // NOTE: We need to keep a pointer to this object to keep it alive for the duration of this function.
    auto const ptr = shared_from_this();
    MainScene::get_instance()->remove_child(ptr);

    for (u32 i = 0; i < components.size(); ++i)
    {
        if (!components[i]->has_been_awaken)
        {
            MainScene::get_instance()->remove_component_to_awake(components[i]);
        }

        if (!components[i]->has_been_started)
        {
            MainScene::get_instance()->remove_component_to_start(components[i]);
        }

        components[i]->set_can_tick(false);

        if (Engine::is_game_running())
            components[i]->set_enabled(false);

        components[i]->uninitialize();
        components[i]->entity = nullptr;
    }

    auto const children_copy = transform->children;
    for (auto const& child : children_copy)
    {
        child->entity.lock()->destroy_immediate();
    }

    if (!transform->parent.expired())
    {
        transform->set_parent(nullptr);
    }
}
