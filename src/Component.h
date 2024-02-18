#pragma once
#include <memory>
#include <string>

class Entity;

class Component : public std::enable_shared_from_this<Component>
{
public:
    virtual ~Component() = default;

    virtual void initialize();
    virtual void awake();
    virtual void start();
    virtual void update();

    virtual std::string get_name() const;
    virtual void draw_editor();

    std::shared_ptr<Entity> entity;

    bool enabled = true;
    bool has_been_started = false;

    void set_can_tick(bool const value);
    bool get_can_tick() const;

private:
    bool can_tick = false;
};
