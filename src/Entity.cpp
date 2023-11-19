#include "Entity.h"

#include "MainScene.h"

Entity::Entity()
{
    transform = std::make_shared<Transform>();
}

// This should return a weak_ptr but it's a pain to use this in that way
// Maybe there is a better pattern
std::shared_ptr<Entity> Entity::create()
{
    auto entity = std::make_shared<Entity>();
    MainScene::get_instance()->add_child(entity);
    return entity;
}
