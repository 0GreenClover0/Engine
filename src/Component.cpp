#include "Component.h"

#include "AK.h"
#include "MainScene.h"

void Component::initialize()
{
}

void Component::awake()
{
}

void Component::start()
{
}

void Component::update()
{
}

std::string Component::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Component::draw_editor()
{
}

void Component::set_can_tick(bool const value)
{
    if (can_tick != value)
    {
        if (value)
            MainScene::get_instance()->tickable_components.emplace_back(shared_from_this());
        else
            AK::swap_and_erase(MainScene::get_instance()->tickable_components, shared_from_this());
    }

    can_tick = value;
}

bool Component::get_can_tick() const
{
    return can_tick;
}
