#include "Component.h"

#include "AK/AK.h"
#include "MainScene.h"

void Component::initialize()
{
}

void Component::uninitialize()
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

void Component::on_enabled()
{
}

void Component::on_disabled()
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
    if (m_can_tick != value)
    {
        if (value)
            MainScene::get_instance()->tickable_components.emplace_back(shared_from_this());
        else
            AK::swap_and_erase(MainScene::get_instance()->tickable_components, shared_from_this());
    }

    m_can_tick = value;
}

bool Component::get_can_tick() const
{
    return m_can_tick;
}

void Component::set_enabled(bool const value)
{
    if (value == m_enabled)
        return;

    m_enabled = value;

    if (value)
    {
        on_enabled();
    }
    else
    {
        on_disabled();
    }
}

bool Component::enabled() const
{
    return m_enabled;
}
