#include "SceneSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

#include "Cube.h"
#include "Ellipse.h"
#include "Entity.h"
#include "Model.h"
#include "Sphere.h"
#include "yaml-cpp-extensions.h"

SceneSerializer::SceneSerializer(std::shared_ptr<Scene> const& scene) : scene(scene)
{
}

void SceneSerializer::serialize_shader(YAML::Emitter& out, std::shared_ptr<Shader> const& shader)
{
    out << YAML::Key << "Shader";
    out << YAML::BeginMap; // Shader

    out << YAML::Key << "VertexPath" << YAML::Value << shader->vertex_path;
    out << YAML::Key << "FragmentPath" << YAML::Value << shader->fragment_path;
    out << YAML::Key << "GeometryPath" << YAML::Value << shader->geometry_path;

    out << YAML::EndMap; // Shader
}

void SceneSerializer::serialize_material_instance(YAML::Emitter& out, std::shared_ptr<Material> const& material)
{
    // TODO: We should not serialize each individual shader and material instances and materials, but rather batch same ones together
    out << YAML::Key << "Material";
    out << YAML::BeginMap; // Material

    serialize_shader(out, material->shader);
    out << YAML::Key << "Color" << YAML::Value << material->color;

    out << YAML::EndMap; // Material
}

std::shared_ptr<Shader> SceneSerializer::deserialize_shader(YAML::Node const& node) const
{
    auto const vertex_path = node["VertexPath"].as<std::string>();
    auto const fragment_path = node["FragmentPath"].as<std::string>();
    auto const geometry_path = node["GeometryPath"].as<std::string>();

    if (geometry_path.empty())
        return Shader::create(vertex_path, fragment_path);

    return Shader::create(vertex_path, fragment_path, geometry_path);
}

std::shared_ptr<Material> SceneSerializer::deserialize_material_instance(YAML::Node const& node) const
{
    auto const shader = deserialize_shader(node["Shader"]);
    auto material = Material::create(shader);
    material->color = node["Color"].as<glm::vec4>();
    return material;
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
        if (auto const model = std::dynamic_pointer_cast<Model>(component); model != nullptr)
        {
            if (auto const ellipse = std::dynamic_pointer_cast<class Ellipse>(component); ellipse != nullptr)
            {
                out << YAML::BeginMap; // ModelComponent
                out << YAML::Key << "ComponentName" << YAML::Value << "EllipseComponent";

                out << YAML::Key << "CenterX" << YAML::Value << ellipse->center_x;
                out << YAML::Key << "CenterZ" << YAML::Value << ellipse->center_z;
                out << YAML::Key << "RadiusX" << YAML::Value << ellipse->radius_x;
                out << YAML::Key << "RadiusZ" << YAML::Value << ellipse->radius_z;
                out << YAML::Key << "SegmentCount" << YAML::Value << ellipse->segment_count;
            }
            else if (auto const sphere = std::dynamic_pointer_cast<Sphere>(component); sphere != nullptr)
            {
                out << YAML::BeginMap; // ModelComponent
                out << YAML::Key << "ComponentName" << YAML::Value << "SphereComponent";

                out << YAML::Key << "Radius" << YAML::Value << sphere->radius;
                out << YAML::Key << "Sectors" << YAML::Value << sphere->sector_count;
                out << YAML::Key << "Stacks" << YAML::Value << sphere->stack_count;
                out << YAML::Key << "TexturePath" << YAML::Value << sphere->texture_path;
            }
            else if (auto const cube = std::dynamic_pointer_cast<class Cube>(component); cube != nullptr)
            {
                out << YAML::BeginMap; // ModelComponent
                out << YAML::Key << "ComponentName" << YAML::Value << "CubeComponent";

                out << YAML::Key << "TexturePath" << YAML::Value << cube->diffuse_texture_path;
            }
            else
            {
                out << YAML::BeginMap; // ModelComponent
                out << YAML::Key << "ComponentName" << YAML::Value << "ModelComponent";
            }

            out << YAML::Key << "ModelPath" << YAML::Value << model->model_path;

            serialize_material_instance(out, model->material);

            out << YAML::EndMap; // ModelComponent
        }
    }
    out << YAML::EndSeq; // Components

    out << YAML::EndMap; // Entity
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
    deserialized_entity->parent_guid = transform["Parent"].as<std::string>();

    auto const components = entity["Components"];

    for (auto it = components.begin(); it != components.end(); ++it)
    {
        YAML::Node const& component = *it;
        auto component_name = component["ComponentName"].as<std::string>();
        if (component_name == "EllipseComponent")
        {
            auto material = deserialize_material_instance(component["Material"]);
            deserialized_entity->add_component<class Ellipse>(
                component["CenterX"].as<float>(),
                component["CenterZ"].as<float>(),
                component["RadiusX"].as<float>(),
                component["RadiusZ"].as<float>(),
                component["SegmentCount"].as<int>(),
                material
            );
        }
        else if (component_name == "SphereComponent")
        {
            auto material = deserialize_material_instance(component["Material"]);
            deserialized_entity->add_component<Sphere>(
                component["Radius"].as<float>(),
                component["Sectors"].as<uint32_t>(),
                component["Stacks"].as<uint32_t>(),
                component["TexturePath"].as<std::string>(),
                material
            );
        }
        else if (component_name == "CubeComponent")
        {
            auto material = deserialize_material_instance(component["Material"]);
            deserialized_entity->add_component<Cube>(
                Cube::create(component["TexturePath"].as<std::string>(), material)
            );
        }
        else if (component_name == "ModelComponent")
        {
            auto material = deserialize_material_instance(component["Material"]);
            deserialized_entity->add_component<Model>(
                Model::create(
                    component["ModelPath"].as<std::string>(),
                    material
                )
            );
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

    for (auto const& entity : scene->entities)
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
            if (entity->parent_guid.empty())
                continue;

            for (auto const& other_entity : deserialized_entities)
            {
                if (entity->parent_guid == other_entity->guid)
                {
                    entity->transform->set_parent(other_entity->transform);
                    break;
                }
            }
        }
    }

    return true;
}
