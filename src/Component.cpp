#include "Component.h"

#include "Entity.h"
#include "AK/AK.h"
#include "MainScene.h"

#include <imgui.h>

Component::Component()
{
    guid = AK::generate_guid();
}

void Component::initialize()
{
}

void Component::uninitialize()
{
}

void Component::reprepare()
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

void Component::destroy_immediate()
{
    assert(entity != nullptr);

    auto const shared = shared_from_this();

    if (!has_been_awaken)
    {
        MainScene::get_instance()->remove_component_to_awake(shared);
    }

    if (!has_been_started)
    {
        MainScene::get_instance()->remove_component_to_start(shared);
    }

    set_can_tick(false);
    set_enabled(false);
    uninitialize();

    AK::swap_and_erase(entity->components, shared);
    entity = nullptr;
}

void Component::draw_editor()
{
    if (ImGui::Button("Remove component", ImVec2(-FLT_MIN, 20.0f)))
    {
        destroy_immediate();
    }
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
