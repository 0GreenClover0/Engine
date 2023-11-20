#include "Entity.h"

#include "MainScene.h"

Entity::Entity(std::string name) : name(std::move(name))
{
}

// This should return a weak_ptr but it's a pain to use this in that way
// Maybe there is a better pattern
std::shared_ptr<Entity> Entity::create(std::string name)
{
    auto entity = std::make_shared<Entity>(name);
    entity->transform = std::make_shared<Transform>(entity);
    MainScene::get_instance()->add_child(entity);
    return entity;
}
