#include "Factory.h"

#include "Entity.h"
#include "Game/LevelController.h"
#include "Model.h"
#include "Player.h"
#include "ResourceManager.h"

std::shared_ptr<Factory> Factory::create()
{
    return std::make_shared<Factory>(AK::Badge<Factory> {});
}

Factory::Factory(AK::Badge<Factory>)
{
}

bool Factory::interact() const
{
    if (Player::get_instance()->packages <= 0)
        return false;

    if (type == FactoryType::Generator)
    {
        Player::get_instance()->flash += 1;
    }
    else if (type == FactoryType::Workshop)
    {
        Player::get_instance()->upgrade_lighthouse();
    }

    Player::get_instance()->packages -= 1;

    return true;
}

void Factory::draw_editor()
{
    Component::draw_editor();

    std::array const factory_types = {"Generator", "Workshop"};
    i32 current_item_index = static_cast<i32>(type);
    if (ImGui::Combo("Factory Type", &current_item_index, factory_types.data(), factory_types.size()))
    {
        type = static_cast<FactoryType>(current_item_index);
    }
}

void Factory::set_type(FactoryType const type)
{
    this->type = type;

    if (!model.expired())
    {
        model.lock()->destroy_immediate();
    }

    auto const standard_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    auto const standard_material = Material::create(standard_shader);

    if (type == FactoryType::Generator)
    {
        model = entity->add_component<Model>(Model::create("./res/models/hubert/floor.gltf", standard_material));
        entity->transform->set_local_scale({0.05f, 0.1f, 0.05f});
    }
    else if (type == FactoryType::Workshop)
    {
        model = entity->add_component<Model>(Model::create("./res/models/lighthouseOld/lighthouse.gltf", standard_material));
        entity->transform->set_local_scale({1.0f, 1.0f, 1.0f});
    }
}
