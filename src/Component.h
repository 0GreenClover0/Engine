#pragma once
#include <memory>

class Entity;

class Component : public std::enable_shared_from_this<Component>
{
public:
    virtual ~Component() = default;
    // TODO: Call these on entities instantiated during these calls.
    // TODO: Order of calling these on entities instantiated during Awake is wrong.
    virtual void initialize();
    virtual void awake();
    virtual void start();
    virtual void update();

    std::shared_ptr<Entity> entity;

    bool enabled = true;
};
