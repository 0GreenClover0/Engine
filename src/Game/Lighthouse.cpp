#include "Lighthouse.h"

#include "Entity.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "Model.h"
#include "ResourceManager.h"

#include <imgui_extensions.h>

#include "Collider2D.h"

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

void Lighthouse::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Light", light);
    ImGuiEx::draw_ptr("Spawn", spawn_position);
}

void Lighthouse::enter()
{
    if (m_is_keeeper_inside)
    {
        Debug::log("We are already inside a lighthouse, why is this being called?", DebugType::Error);
        return;
    }

    m_is_keeeper_inside = true;
    light.lock()->set_enabled(true);
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

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    auto const keeper = Entity::create("keeper");
    keeper->add_component(Model::create("./res/models/keeperInHovercraft/keeperInHovercraft.gltf", standard_material));
    keeper->add_component(Collider2D::create(0.1f));
    auto const keeper_comp = keeper->add_component(LighthouseKeeper::create());
    keeper_comp->lighthouse = static_pointer_cast<Lighthouse>(shared_from_this());
    keeper_comp->port = port;

    keeper->transform->set_local_position(spawn_position.lock()->transform->get_position());
}
