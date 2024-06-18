#include "Lighthouse.h"

#include "Collider2D.h"
#include "Entity.h"
#include "Floater.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "LevelController.h"
#include "Model.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"

#if EDITOR
#include "imgui_extensions.h"
#endif

std::shared_ptr<Lighthouse> Lighthouse::create()
{
    return std::make_shared<Lighthouse>(AK::Badge<Lighthouse> {});
}

Lighthouse::Lighthouse(AK::Badge<Lighthouse>)
{
}

void Lighthouse::awake()
{
    set_can_tick(true);
}

void Lighthouse::update()
{
    if (m_is_keeeper_inside && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        exit();
    }
}

#if EDITOR
void Lighthouse::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Light", light);
    ImGuiEx::draw_ptr("Spawn", spawn_position);
    ImGuiEx::draw_ptr("Water", water);
}
#endif

void Lighthouse::enter()
{
    if (m_is_keeeper_inside)
    {
        Debug::log("We are already inside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    m_is_keeeper_inside = true;
    light.lock()->set_enabled(true);
    light.lock()->spotlight.lock()->set_enabled(true);
    light.lock()->entity->get_component<Sphere>()->set_enabled(true);
}

void Lighthouse::exit()
{
    if (!m_is_keeeper_inside)
    {
        Debug::log("We are already outside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    m_is_keeeper_inside = false;
    light.lock()->set_enabled(false);
    light.lock()->spotlight.lock()->set_enabled(false);
    light.lock()->entity->get_component<Sphere>()->set_enabled(false);

    auto const keeper = SceneSerializer::load_prefab("Keeper");

    keeper->transform->set_local_position(spawn_position.lock()->transform->get_position());
    keeper->get_component<LighthouseKeeper>()->port = LevelController::get_instance()->port;
    keeper->get_component<LighthouseKeeper>()->lighthouse = std::static_pointer_cast<Lighthouse>(shared_from_this());
    keeper->get_component<Floater>()->water = water;
}
