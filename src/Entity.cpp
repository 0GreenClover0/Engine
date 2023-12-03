#include "Entity.h"

#include "MainScene.h"
#include "AK.h"

Entity::Entity(std::string name) : name(std::move(name))
{
}

// This should return a weak_ptr but it's a pain to use this in that way
// Maybe there is a better pattern
std::shared_ptr<Entity> Entity::create(std::string const& name)
{
    auto entity = std::make_shared<Entity>(name);
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
    auto entity = std::make_shared<Entity>(name);
    entity->guid = guid;
    std::hash<std::string> constexpr hasher;
    entity->hashed_guid = hasher(entity->guid);
    entity->transform = std::make_shared<Transform>(entity);
    MainScene::get_instance()->add_child(entity);
    return entity;
}
