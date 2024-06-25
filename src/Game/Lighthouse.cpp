#include "Lighthouse.h"

#include "Collider2D.h"
#include "Entity.h"
#include "Floater.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "GameController.h"
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
    m_has_keeper_exited_this_frame = false;
    if (m_is_keeeper_inside && !m_has_keeper_entered_this_frame && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        exit();
    }
    m_has_keeper_entered_this_frame = false;
}

void Lighthouse::start()
{
    if (GameController::get_instance()->get_level_number() != 0)
    {
        keeper = SceneSerializer::load_prefab("Keeper");
        keeper->get_component<LighthouseKeeper>()->set_is_driving(false);
        keeper->get_component<Collider2D>()->set_enabled(false);
        keeper->get_component<Floater>()->set_enabled(false);
        keeper->get_component<Model>()->model_path = "./res/models/hovercraft/hovercraft.gltf";
        keeper->get_component<Model>()->reprepare();
        keeper->get_component<LighthouseKeeper>()->port = LevelController::get_instance()->port;
        keeper->get_component<LighthouseKeeper>()->lighthouse = std::static_pointer_cast<Lighthouse>(shared_from_this());
        keeper->get_component<Floater>()->water = water;
        keeper->transform->set_parent(entity->transform);
        keeper->transform->set_local_position(spawn_position.lock()->transform->get_local_position() + glm::vec3(0.0f, 0.07f, 0.0f));
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

void Lighthouse::turn_light(bool const value) const
{
    light.lock()->set_enabled(value);
    light.lock()->spotlight.lock()->set_enabled(value);
    light.lock()->entity->get_component<Sphere>()->set_enabled(value);
}

bool Lighthouse::is_keeper_inside() const
{
    return m_is_keeeper_inside;
}

bool Lighthouse::check_if_keeper_is_inside() const
{
    return m_is_keeeper_inside;
}

void Lighthouse::enter()
{
    if (m_is_keeeper_inside || m_has_keeper_exited_this_frame)
    {
        Debug::log("We are already inside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    m_has_keeper_entered_this_frame = true;
    m_is_keeeper_inside = true;
    light.lock()->set_enabled(true);
    light.lock()->spotlight.lock()->set_enabled(true);
    light.lock()->entity->get_component<Sphere>()->set_enabled(true);

    keeper->get_component<LighthouseKeeper>()->set_is_driving(false);
    keeper->get_component<Model>()->model_path = "./res/models/hovercraft/hovercraft.gltf";
    keeper->get_component<Model>()->reprepare();

    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::KeeperEnteredLighthouse);
}

void Lighthouse::exit()
{
    if (!LevelController::get_instance()->get_exiting_lighthouse())
    {
        return;
    }

    if (!m_is_keeeper_inside)
    {
        Debug::log("We are already outside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    m_is_keeeper_inside = false;
    m_has_keeper_exited_this_frame = true;
    light.lock()->set_enabled(false);
    light.lock()->spotlight.lock()->set_enabled(false);
    light.lock()->entity->get_component<Sphere>()->set_enabled(false);

    keeper->get_component<LighthouseKeeper>()->set_is_driving(true);
    keeper->get_component<Collider2D>()->set_enabled(true);
    keeper->get_component<Model>()->model_path = "./res/models/keeperInHovercraft/keeper.gltf";
    keeper->get_component<Model>()->reprepare();

    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::KeeperLeftLighthouse);
}
