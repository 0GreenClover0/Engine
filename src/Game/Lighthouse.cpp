#include "Lighthouse.h"

#include "Camera.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Floater.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "GameController.h"
#include "HovercraftWithoutKeeper.h"
#include "LevelController.h"
#include "Model.h"
#include "ResourceManager.h"
#include "SceneSerializer.h"

#include <glm/gtc/random.hpp>

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
    if (m_is_keeeper_inside && !m_has_keeper_entered_this_frame && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        exit();
    }

    m_has_keeper_entered_this_frame = false;
}

void Lighthouse::start()
{
    spawn_hovercraft();
    spawn_fake_packages(Player::get_instance()->packages, m_hovercraft.lock()->transform);
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

void Lighthouse::enter()
{
    if (m_is_keeeper_inside)
    {
        Debug::log("We are already inside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    glm::vec3 const position = m_keeper.lock()->transform->get_position();
    glm::vec3 const rotation = glm::degrees(glm::eulerAngles(m_keeper.lock()->transform->get_rotation()));

    spawn_hovercraft(position, rotation);
    spawn_fake_packages(m_keeper.lock()->get_component<LighthouseKeeper>()->packages.size(), m_hovercraft.lock()->transform);
    m_hovercraft.lock()->get_component<HovercraftWithoutKeeper>()->speed = m_keeper.lock()->get_component<LighthouseKeeper>()->get_speed();

    m_has_keeper_entered_this_frame = true;
    m_is_keeeper_inside = true;
    light.lock()->set_enabled(true);
    light.lock()->spotlight.lock()->set_enabled(true);
    light.lock()->entity->get_component<Sphere>()->set_enabled(true);

    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::KeeperEnteredLighthouse);

    auto const enter_sound = Sound::play_sound_at_location("./res/audio/pickup/enter_latarnia.wav", entity->transform->get_position(),
                                                           Camera::get_main_camera()->get_position());
    enter_sound->set_volume(15.0f);
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

    glm::vec3 const position = m_hovercraft.lock()->transform->get_position();
    glm::vec3 const rotation = glm::degrees(glm::eulerAngles(m_hovercraft.lock()->transform->get_rotation()));

    despawn_hovercraft();

    m_is_keeeper_inside = false;
    light.lock()->set_enabled(false);
    light.lock()->spotlight.lock()->set_enabled(false);
    light.lock()->entity->get_component<Sphere>()->set_enabled(false);

    auto const keeper = SceneSerializer::load_prefab("Keeper");

    keeper->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);
    keeper->transform->set_position(position);
    keeper->transform->set_rotation(rotation);
    keeper->get_component<LighthouseKeeper>()->port = LevelController::get_instance()->port;
    keeper->get_component<LighthouseKeeper>()->lighthouse = std::static_pointer_cast<Lighthouse>(shared_from_this());
    keeper->get_component<Floater>()->water = water;
    m_keeper = keeper;

    LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::KeeperLeftLighthouse);

    auto const exit_sound = Sound::play_sound_at_location("./res/audio/pickup/enter_latarnia.wav", entity->transform->get_position(),
                                                          Camera::get_main_camera()->get_position());
    exit_sound->set_volume(15.0f);
}

void Lighthouse::spawn_hovercraft()
{
    auto const hovercraft_locked = SceneSerializer::load_prefab("Hovercraft");
    m_hovercraft = hovercraft_locked;

    if (GameController::get_instance()->next_scene.expired())
    {
        hovercraft_locked->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);
    }
    else
    {
        hovercraft_locked->transform->set_parent(GameController::get_instance()->next_scene.lock()->transform);
    }

    hovercraft_locked->transform->set_position(spawn_position.lock()->transform->get_position() + glm::vec3(0.0f, 0.07f, 0.0f));
}

void Lighthouse::spawn_hovercraft(glm::vec3 const& position, glm::vec3 const& euler_angles)
{
    auto const hovercraft_locked = SceneSerializer::load_prefab("Hovercraft");
    m_hovercraft = hovercraft_locked;
    hovercraft_locked->transform->set_parent(GameController::get_instance()->current_scene.lock()->transform);
    hovercraft_locked->transform->set_position(position + glm::vec3(0.0f, 0.05f, 0.0f));
    hovercraft_locked->transform->set_rotation(euler_angles);
}

void Lighthouse::despawn_hovercraft()
{
    if (m_hovercraft.expired())
    {
        Debug::log("Trying to despawn non existent hovercraft.", DebugType::Error);
        return;
    }

    m_hovercraft.lock()->destroy_immediate();
}

void Lighthouse::spawn_fake_packages(u32 const packages_count, std::shared_ptr<Transform> const& parent) const
{
    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    std::shared_ptr<Transform> last_package = {};
    for (u32 i = 0; i < packages_count; ++i)
    {
        auto const package = Entity::create("Package");
        package->add_component(Model::create("./res/models/package/package.gltf", standard_material));

        if (i > 0)
        {
            package->transform->set_parent(last_package);
            float const x = package->transform->parent.lock()->get_local_position().x;
            float const z = package->transform->parent.lock()->get_local_position().z;
            package->transform->set_local_position(
                glm::vec3(glm::linearRand(-0.015f, 0.015f) - x, 0.13f, glm::linearRand(-0.02f, 0.02f) - z));
        }
        else
        {
            package->transform->set_parent(parent);
            last_package = package->transform;
        }
    }
}
