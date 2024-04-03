#include "Entity.h"

#include "MainScene.h"
#include "AK/AK.h"

Entity::Entity(AK::Badge<Entity>, std::string name) : name(std::move(name))
{
}

// This should return a weak_ptr but it's a pain to use this in that way
// Maybe there is a better pattern
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

// This should return a weak_ptr but it's a pain to use this in that way
// Maybe there is a better pattern
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

void Entity::destroy_immediate()
{
    MainScene::get_instance()->remove_child(shared_from_this());

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
        components[i]->uninitialize();
        components[i]->entity = nullptr;
    }
}
