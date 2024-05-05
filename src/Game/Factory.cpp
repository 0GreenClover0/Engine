#include "Factory.h"

#include "Entity.h"
#include "Model.h"
#include "ResourceManager.h"
#include "Game/GameController.h"

std::shared_ptr<Factory> Factory::create()
{
    return std::make_shared<Factory>(AK::Badge<Factory> {});
}

Factory::Factory(AK::Badge<Factory>)
{
}

bool Factory::interact() const
{
    if (GameController::get_instance()->packages <= 0)
        return false;

    if (type == FactoryType::Generator)
    {
        GameController::get_instance()->flash += 1;
    }
    else if (type == FactoryType::Workshop)
    {
        GameController::get_instance()->lighthouse_level += 1;
    }

    GameController::get_instance()->packages -= 1;

    return true;
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
        entity->transform->set_local_scale({ 0.05f, 0.1f, 0.05f });
    }
    else if (type == FactoryType::Workshop)
    {
        model = entity->add_component<Model>(Model::create("./res/models/lighthouse/lighthouse.gltf", standard_material));
        entity->transform->set_local_scale({ 1.0f, 1.0f, 1.0f });
    }
}
