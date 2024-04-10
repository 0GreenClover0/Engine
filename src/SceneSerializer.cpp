#include "SceneSerializer.h"

#include <fstream>
#include <iostream>
#include <yaml-cpp/yaml.h>

#include "Entity.h"
#include "ShaderFactory.h"
#include "yaml-cpp-extensions.h"
// # Put new header here

SceneSerializer::SceneSerializer(std::shared_ptr<Scene> const& scene) : m_scene(scene)
{
}

void SceneSerializer::auto_serialize_component(YAML::Emitter& out, std::shared_ptr<Component> const& component)
{
    // # Auto serialization start
    {
        std::cout << "Error. Serialization of component " << component->get_name() << " failed." << "\n";
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
