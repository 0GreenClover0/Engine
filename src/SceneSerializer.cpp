#include "SceneSerializer.h"

#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "ShaderFactory.h"
#include "yaml-cpp-extensions.h"
#include "Camera.h"
#include "Collider2D.h"
#include "Drawable.h"
#include "Model.h"
#include "Cube.h"
#include "Ellipse.h"
#include "Sphere.h"
#include "Sprite.h"
#include "ScreenText.h"
#include "ExampleDynamicText.h"
#include "ExampleUIBar.h"
#include "Light.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Sound.h"
#include "SoundListener.h"
#include "Game/Player/PlayerInput.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "Game/Ship.h"
// # Put new header here

SceneSerializer::SceneSerializer(std::shared_ptr<Scene> const& scene) : m_scene(scene)
{
}

std::shared_ptr<SceneSerializer> SceneSerializer::get_instance()
{
    return m_instance;
}

void SceneSerializer::set_instance(std::shared_ptr<SceneSerializer> const& instance)
{
    m_instance = instance;
}

void SceneSerializer::auto_serialize_component(YAML::Emitter& out, std::shared_ptr<Component> const& component)
{
    // # Auto serialization start
    if (auto const camera = std::dynamic_pointer_cast<class Camera>(component); camera != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "CameraComponent";
        out << YAML::Key << "guid" << YAML::Value << camera->guid;
        out << YAML::Key << "width" << YAML::Value << camera->width;
        out << YAML::Key << "height" << YAML::Value << camera->height;
        out << YAML::Key << "fov" << YAML::Value << camera->fov;
        out << YAML::Key << "near_plane" << YAML::Value << camera->near_plane;
        out << YAML::Key << "far_plane" << YAML::Value << camera->far_plane;
        out << YAML::EndMap;
    }
    else
    if (auto const collider2d = std::dynamic_pointer_cast<class Collider2D>(component); collider2d != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "Collider2DComponent";
        out << YAML::Key << "guid" << YAML::Value << collider2d->guid;
        out << YAML::EndMap;
    }
    else
    if (auto const drawable = std::dynamic_pointer_cast<class Drawable>(component); drawable != nullptr)
    {
        out << YAML::BeginMap;
        // # Put new Drawable kid here
        if (auto const screentext = std::dynamic_pointer_cast<class ScreenText>(component); screentext != nullptr)
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "ScreenTextComponent";
            out << YAML::Key << "guid" << YAML::Value << screentext->guid;
            out << YAML::Key << "text" << YAML::Value << screentext->text;
            out << YAML::Key << "position" << YAML::Value << screentext->position;
            out << YAML::Key << "font_size" << YAML::Value << screentext->font_size;
            out << YAML::Key << "color" << YAML::Value << screentext->color;
            out << YAML::Key << "flags" << YAML::Value << screentext->flags;
        }
        else
        if (auto const model = std::dynamic_pointer_cast<class Model>(component); model != nullptr)
        {
            // # Put new Model kid here
            if (auto const sprite = std::dynamic_pointer_cast<class Sprite>(component); sprite != nullptr)
            {
                out << YAML::Key << "ComponentName" << YAML::Value << "SpriteComponent";
                out << YAML::Key << "guid" << YAML::Value << sprite->guid;
                out << YAML::Key << "diffuse_texture_path" << YAML::Value << sprite->diffuse_texture_path;
            }
            else
            if (auto const sphere = std::dynamic_pointer_cast<class Sphere>(component); sphere != nullptr)
            {
                out << YAML::Key << "ComponentName" << YAML::Value << "SphereComponent";
                out << YAML::Key << "guid" << YAML::Value << sphere->guid;
                out << YAML::Key << "sector_count" << YAML::Value << sphere->sector_count;
                out << YAML::Key << "stack_count" << YAML::Value << sphere->stack_count;
                out << YAML::Key << "texture_path" << YAML::Value << sphere->texture_path;
                out << YAML::Key << "radius" << YAML::Value << sphere->radius;
            }
            else
            if (auto const ellipse = std::dynamic_pointer_cast<class Ellipse>(component); ellipse != nullptr)
            {
                out << YAML::Key << "ComponentName" << YAML::Value << "EllipseComponent";
                out << YAML::Key << "guid" << YAML::Value << ellipse->guid;
                out << YAML::Key << "center_x" << YAML::Value << ellipse->center_x;
                out << YAML::Key << "center_z" << YAML::Value << ellipse->center_z;
                out << YAML::Key << "radius_x" << YAML::Value << ellipse->radius_x;
                out << YAML::Key << "radius_z" << YAML::Value << ellipse->radius_z;
                out << YAML::Key << "segment_count" << YAML::Value << ellipse->segment_count;
            }
            else
            if (auto const cube = std::dynamic_pointer_cast<class Cube>(component); cube != nullptr)
            {
                out << YAML::Key << "ComponentName" << YAML::Value << "CubeComponent";
                out << YAML::Key << "guid" << YAML::Value << cube->guid;
                out << YAML::Key << "diffuse_texture_path" << YAML::Value << cube->diffuse_texture_path;
                out << YAML::Key << "specular_texture_path" << YAML::Value << cube->specular_texture_path;
            }
            else
            {
                out << YAML::Key << "ComponentName" << YAML::Value << "ModelComponent";
                out << YAML::Key << "guid" << YAML::Value << model->guid;
            }
            out << YAML::Key << "model_path" << YAML::Value << model->model_path;
        }
        else
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "DrawableComponent";
            out << YAML::Key << "guid" << YAML::Value << drawable->guid;
        }
        out << YAML::Key << "material" << YAML::Value << drawable->material;
        out << YAML::EndMap;
    }
    else
    if (auto const exampledynamictext = std::dynamic_pointer_cast<class ExampleDynamicText>(component); exampledynamictext != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "ExampleDynamicTextComponent";
        out << YAML::Key << "guid" << YAML::Value << exampledynamictext->guid;
        out << YAML::EndMap;
    }
    else
    if (auto const exampleuibar = std::dynamic_pointer_cast<class ExampleUIBar>(component); exampleuibar != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "ExampleUIBarComponent";
        out << YAML::Key << "guid" << YAML::Value << exampleuibar->guid;
        out << YAML::Key << "value" << YAML::Value << exampleuibar->value;
        out << YAML::EndMap;
    }
    else
    if (auto const light = std::dynamic_pointer_cast<class Light>(component); light != nullptr)
    {
        out << YAML::BeginMap;
        // # Put new Light kid here
        if (auto const spotlight = std::dynamic_pointer_cast<class SpotLight>(component); spotlight != nullptr)
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "SpotLightComponent";
            out << YAML::Key << "guid" << YAML::Value << spotlight->guid;
            out << YAML::Key << "constant" << YAML::Value << spotlight->constant;
            out << YAML::Key << "linear" << YAML::Value << spotlight->linear;
            out << YAML::Key << "quadratic" << YAML::Value << spotlight->quadratic;
            out << YAML::Key << "cut_off" << YAML::Value << spotlight->cut_off;
            out << YAML::Key << "outer_cut_off" << YAML::Value << spotlight->outer_cut_off;
        }
        else
        if (auto const pointlight = std::dynamic_pointer_cast<class PointLight>(component); pointlight != nullptr)
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "PointLightComponent";
            out << YAML::Key << "guid" << YAML::Value << pointlight->guid;
            out << YAML::Key << "constant" << YAML::Value << pointlight->constant;
            out << YAML::Key << "linear" << YAML::Value << pointlight->linear;
            out << YAML::Key << "quadratic" << YAML::Value << pointlight->quadratic;
        }
        else
        if (auto const directionallight = std::dynamic_pointer_cast<class DirectionalLight>(component); directionallight != nullptr)
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "DirectionalLightComponent";
            out << YAML::Key << "guid" << YAML::Value << directionallight->guid;
        }
        else
        {
            out << YAML::Key << "ComponentName" << YAML::Value << "LightComponent";
            out << YAML::Key << "guid" << YAML::Value << light->guid;
        }
        out << YAML::Key << "ambient" << YAML::Value << light->ambient;
        out << YAML::Key << "diffuse" << YAML::Value << light->diffuse;
        out << YAML::Key << "specular" << YAML::Value << light->specular;
        out << YAML::EndMap;
    }
    else
    if (auto const sound = std::dynamic_pointer_cast<class Sound>(component); sound != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "SoundComponent";
        out << YAML::Key << "guid" << YAML::Value << sound->guid;
        out << YAML::EndMap;
    }
    else
    if (auto const soundlistener = std::dynamic_pointer_cast<class SoundListener>(component); soundlistener != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "SoundListenerComponent";
        out << YAML::Key << "guid" << YAML::Value << soundlistener->guid;
        out << YAML::EndMap;
    }
    else
    if (auto const lighthousekeeper = std::dynamic_pointer_cast<class LighthouseKeeper>(component); lighthousekeeper != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "LighthouseKeeperComponent";
        out << YAML::Key << "guid" << YAML::Value << lighthousekeeper->guid;
        out << YAML::Key << "deceleration" << YAML::Value << lighthousekeeper->deceleration;
        out << YAML::EndMap;
    }
    else
    if (auto const lighthouselight = std::dynamic_pointer_cast<class LighthouseLight>(component); lighthouselight != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "LighthouseLightComponent";
        out << YAML::Key << "guid" << YAML::Value << lighthouselight->guid;
        out << YAML::EndMap;
    }
    else
    if (auto const ship = std::dynamic_pointer_cast<class Ship>(component); ship != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "ShipComponent";
        out << YAML::Key << "guid" << YAML::Value << ship->guid;
        out << YAML::Key << "turn_speed" << YAML::Value << ship->turn_speed;
        out << YAML::Key << "visibility_range" << YAML::Value << ship->visibility_range;
        out << YAML::EndMap;
    }
    else
    if (auto const playerinput = std::dynamic_pointer_cast<class PlayerInput>(component); playerinput != nullptr)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ComponentName" << YAML::Value << "PlayerInputComponent";
        out << YAML::Key << "guid" << YAML::Value << playerinput->guid;
        out << YAML::Key << "player_speed" << YAML::Value << playerinput->player_speed;
        out << YAML::Key << "camera_speed" << YAML::Value << playerinput->camera_speed;
        out << YAML::EndMap;
    }
    else
    {
        // NOTE: This only returns unmangled name while using the MSVC compiler
        std::string const name = typeid(*component).name();
        std::cout << "Error. Serialization of component " << name.substr(6) << " failed." << "\n";
    }
    // # Put new serialization here
}

void SceneSerializer::serialize_entity(YAML::Emitter& out, std::shared_ptr<Entity> const& entity)
{
    out << YAML::BeginMap; // Entity
    out << YAML::Key << "Entity" << YAML::Value << entity->guid;
    out << YAML::Key << "Name" << YAML::Value << entity->name;

    {
        out << YAML::Key << "TransformComponent";
        out << YAML::BeginMap; // TransformComponent

        out << YAML::Key << "Translation" << YAML::Value << entity->transform->get_local_position();
        out << YAML::Key << "Rotation" << YAML::Value << entity->transform->get_euler_angles();
        out << YAML::Key << "Scale" << YAML::Value << entity->transform->get_local_scale();

        if (!entity->transform->parent.expired())
            out << YAML::Key << "Parent" << YAML::Value << entity->transform->parent.lock()->entity.lock()->guid;
        else
            out << YAML::Key << "Parent" << YAML::Value << "";

        out << YAML::EndMap; // TransformComponent
    }

    out << YAML::Key << "Components";
    out << YAML::BeginSeq; // Components
    for (auto const& component : entity->components)
    {
        if (false)
        {
            // Custom serialization here
        }
        else
        {
            auto_serialize_component(out, component);
        }
    }
    out << YAML::EndSeq; // Components

    out << YAML::EndMap; // Entity
}

void SceneSerializer::auto_deserialize_component(YAML::Node const& component, std::shared_ptr<Entity> const& deserialized_entity) const
{
    auto component_name = component["ComponentName"].as<std::string>();
    // # Auto deserialization start
    if (component_name == "CameraComponent")
    {
        auto const deserialized_component = Camera::create();
        deserialized_component->width = component["width"].as<float>();
        deserialized_component->height = component["height"].as<float>();
        deserialized_component->fov = component["fov"].as<float>();
        deserialized_component->near_plane = component["near_plane"].as<float>();
        deserialized_component->far_plane = component["far_plane"].as<float>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "Collider2DComponent")
    {
        auto const deserialized_component = Collider2D::create();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "ModelComponent")
    {
        auto const deserialized_component = Model::create();
        deserialized_component->model_path = component["model_path"].as<std::string>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "CubeComponent")
    {
        auto const deserialized_component = Cube::create();
        deserialized_component->diffuse_texture_path = component["diffuse_texture_path"].as<std::string>();
        deserialized_component->specular_texture_path = component["specular_texture_path"].as<std::string>();
        deserialized_component->model_path = component["model_path"].as<std::string>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "EllipseComponent")
    {
        auto const deserialized_component = Ellipse::create();
        deserialized_component->center_x = component["center_x"].as<float>();
        deserialized_component->center_z = component["center_z"].as<float>();
        deserialized_component->radius_x = component["radius_x"].as<float>();
        deserialized_component->radius_z = component["radius_z"].as<float>();
        deserialized_component->segment_count = component["segment_count"].as<i32>();
        deserialized_component->model_path = component["model_path"].as<std::string>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "SphereComponent")
    {
        auto const deserialized_component = Sphere::create();
        deserialized_component->sector_count = component["sector_count"].as<u32>();
        deserialized_component->stack_count = component["stack_count"].as<u32>();
        deserialized_component->texture_path = component["texture_path"].as<std::string>();
        deserialized_component->radius = component["radius"].as<float>();
        deserialized_component->model_path = component["model_path"].as<std::string>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "SpriteComponent")
    {
        auto const deserialized_component = Sprite::create();
        deserialized_component->diffuse_texture_path = component["diffuse_texture_path"].as<std::string>();
        deserialized_component->model_path = component["model_path"].as<std::string>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "ScreenTextComponent")
    {
        auto const deserialized_component = ScreenText::create();
        deserialized_component->text = component["text"].as<std::wstring>();
        deserialized_component->position = component["position"].as<glm::vec2>();
        deserialized_component->font_size = component["font_size"].as<float>();
        deserialized_component->color = component["color"].as<u32>();
        deserialized_component->flags = component["flags"].as<u16>();
        deserialized_component->material = component["material"].as<std::shared_ptr<Material>>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "ExampleDynamicTextComponent")
    {
        auto const deserialized_component = ExampleDynamicText::create();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "ExampleUIBarComponent")
    {
        auto const deserialized_component = ExampleUIBar::create();
        deserialized_component->value = component["value"].as<float>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "LightComponent")
    {
        auto const deserialized_component = Light::create();
        deserialized_component->ambient = component["ambient"].as<glm::vec3>();
        deserialized_component->diffuse = component["diffuse"].as<glm::vec3>();
        deserialized_component->specular = component["specular"].as<glm::vec3>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "DirectionalLightComponent")
    {
        auto const deserialized_component = DirectionalLight::create();
        deserialized_component->ambient = component["ambient"].as<glm::vec3>();
        deserialized_component->diffuse = component["diffuse"].as<glm::vec3>();
        deserialized_component->specular = component["specular"].as<glm::vec3>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "PointLightComponent")
    {
        auto const deserialized_component = PointLight::create();
        deserialized_component->constant = component["constant"].as<float>();
        deserialized_component->linear = component["linear"].as<float>();
        deserialized_component->quadratic = component["quadratic"].as<float>();
        deserialized_component->ambient = component["ambient"].as<glm::vec3>();
        deserialized_component->diffuse = component["diffuse"].as<glm::vec3>();
        deserialized_component->specular = component["specular"].as<glm::vec3>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "SpotLightComponent")
    {
        auto const deserialized_component = SpotLight::create();
        deserialized_component->constant = component["constant"].as<float>();
        deserialized_component->linear = component["linear"].as<float>();
        deserialized_component->quadratic = component["quadratic"].as<float>();
        deserialized_component->cut_off = component["cut_off"].as<float>();
        deserialized_component->outer_cut_off = component["outer_cut_off"].as<float>();
        deserialized_component->ambient = component["ambient"].as<glm::vec3>();
        deserialized_component->diffuse = component["diffuse"].as<glm::vec3>();
        deserialized_component->specular = component["specular"].as<glm::vec3>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "LighthouseKeeperComponent")
    {
        auto const deserialized_component = LighthouseKeeper::create();
        deserialized_component->maximum_speed = component["maximum_speed"].as<float>();
        deserialized_component->deceleration = component["deceleration"].as<float>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "LighthouseLightComponent")
    {
        auto const deserialized_component = LighthouseLight::create();
        deserialized_component->range = component["range"].as<float>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "ShipComponent")
    {
        auto const deserialized_component = Ship::create();
        deserialized_component->minimum_speed = component["minimum_speed"].as<float>();
        deserialized_component->maximum_speed = component["maximum_speed"].as<float>();
        deserialized_component->turn_speed = component["turn_speed"].as<float>();
        deserialized_component->visibility_range = component["visibility_range"].as<int>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "SoundComponent")
    {
        auto const deserialized_component = Sound::create();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "SoundListenerComponent")
    {
        auto const deserialized_component = SoundListener::create();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    if (component_name == "PlayerInputComponent")
    {
        auto const deserialized_component = PlayerInput::create();
        deserialized_component->player_speed = component["player_speed"].as<float>();
        deserialized_component->camera_speed = component["camera_speed"].as<float>();
        deserialized_entity->add_component(deserialized_component);
        deserialized_component->reprepare();
    }
    else
    {
        std::cout << "Error. Deserialization of component " << component_name << " failed." << "\n";
    }
    // # Put new deserialization here
}

std::shared_ptr<Entity> SceneSerializer::deserialize_entity(YAML::Node const& entity) const
{
    auto const entity_node = entity["Entity"];
    if (!entity_node)
    {
        std::cout << "Deserialization of a scene failed. Broken entity. No guid present." << "\n";
        return nullptr;
    }
    auto const guid = entity_node.as<std::string>();

    auto const name_node = entity["Name"];
    if (!name_node)
    {
        std::cout << "Deserialization of a scene failed. Broken entity. No name present." << "\n";
        return nullptr;
    }
    auto const name = name_node.as<std::string>();

    std::shared_ptr<Entity> deserialized_entity = Entity::create(guid, name);

    auto const transform = entity["TransformComponent"];
    if (!transform)
    {
        std::cout << "Deserialization of a scene failed. Broken entity. No transform present." << "\n";
        return nullptr;
    }

    deserialized_entity->transform->set_local_position(transform["Translation"].as<glm::vec3>());
    deserialized_entity->transform->set_euler_angles(transform["Rotation"].as<glm::vec3>());
    deserialized_entity->transform->set_local_scale(transform["Scale"].as<glm::vec3>());
    deserialized_entity->m_parent_guid = transform["Parent"].as<std::string>();

    auto const components = entity["Components"];

    for (auto it = components.begin(); it != components.end(); ++it)
    {
        YAML::Node const& component = *it;
        auto component_name = component["ComponentName"].as<std::string>();
        if (false)
        {
            // Custom deserialization here
        }
        else
        {
            auto_deserialize_component(component, deserialized_entity);
        }
    }

    return deserialized_entity;
}

void SceneSerializer::serialize(std::string const& file_path) const
{
    YAML::Emitter out;
    out << YAML::BeginMap;
    out << YAML::Key << "Scene" << YAML::Value << "Untitled";
    out << YAML::Key << "Entities";
    out << YAML::Value << YAML::BeginSeq;

    for (auto const& entity : m_scene->entities)
    {
        serialize_entity(out, entity);
    }

    out << YAML::EndSeq;
    out << YAML::EndMap;

    std::ofstream scene_file(file_path);

    if (!scene_file.is_open())
    {
        std::cout << "Could not create a scene file: " << file_path << "\n";
        return;
    }

    scene_file << out.c_str();
    scene_file.close();
}

bool SceneSerializer::deserialize(std::string const& file_path) const
{
    std::ifstream scene_file(file_path);

    if (!scene_file.is_open())
    {
        std::cout << "Could not open a scene file: " << file_path << "\n";
        return false;
    }

    std::stringstream stream;
    stream << scene_file.rdbuf();
    scene_file.close();

    YAML::Node data = YAML::Load(stream.str());

    if (!data["Scene"])
        return false;

    auto const scene_name = data["Scene"].as<std::string>();
    std::cout << "Deserializing scene " << scene_name << "\n";

    if (auto const entities = data["Entities"])
    {
        std::vector<std::shared_ptr<Entity>> deserialized_entities = {};
        deserialized_entities.reserve(entities.size());

        // First pass. Create all entities.
        for (auto const entity : entities)
        {
            auto const deserialized_entity = deserialize_entity(entity);
            if (deserialized_entity == nullptr)
                return false;

            deserialized_entities.emplace_back(deserialized_entity);
        }

        // Second pass. Assign appropriate parent for each entity.
        // TODO: Create a map of entity : guid to save performance?
        for (auto const& entity : deserialized_entities)
        {
            if (entity->m_parent_guid.empty())
                continue;

            for (auto const& other_entity : deserialized_entities)
            {
                if (entity->m_parent_guid == other_entity->guid)
                {
                    entity->transform->set_parent(other_entity->transform);
                    break;
                }
            }
        }
    }

    return true;
}
