#include "Component.h"

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
