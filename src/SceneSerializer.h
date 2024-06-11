#pragma once

#include <string>
#include <unordered_map>
#include <yaml-cpp/node/node.h>

#include "Material.h"
#include "Scene.h"

namespace YAML
{
class Emitter;
}

enum class DeserializationMode
{
    Normal,
    InjectFromFile, // Tries to deserialize entities from a file into an existing scene. All guids are replaced with new ones.
};

class SceneSerializer
{
public:
    explicit SceneSerializer(std::shared_ptr<Scene> const& scene);

    static std::shared_ptr<SceneSerializer> get_instance();
    static void set_instance(std::shared_ptr<SceneSerializer> const& instance);

    [[nodiscard]] std::shared_ptr<Component> get_from_pool(std::string const& guid) const;
    [[nodiscard]] std::shared_ptr<Entity> get_entity_from_pool(std::string const& guid) const;

    void serialize_this_entity(std::shared_ptr<Entity> const& entity, std::string const& file_path) const;
    std::shared_ptr<Entity> deserialize_this_entity(std::string const& file_path);

    void serialize(std::string const& file_path) const;
    bool deserialize(std::string const& file_path);

    static std::shared_ptr<Entity> load_prefab(std::string const& prefab_name);

private:
    static void serialize_entity(YAML::Emitter& out, std::shared_ptr<Entity> const& entity);
    static void serialize_entity_recursively(YAML::Emitter& out, std::shared_ptr<Entity> const& entity);
    static void auto_serialize_component(YAML::Emitter& out, std::shared_ptr<Component> const& component);
    void auto_deserialize_component(YAML::Node const& component, std::shared_ptr<Entity> const& deserialized_entity, bool const first_pass);

    void deserialize_components(YAML::Node const& entity_node, std::shared_ptr<Entity> const& deserialized_entity, bool const first_pass);

    [[nodiscard]] std::shared_ptr<Entity> deserialize_entity_first_pass(YAML::Node const& entity);
    void deserialize_entity_second_pass(YAML::Node const& entity, std::shared_ptr<Entity> const& deserialized_entity);

    std::vector<std::shared_ptr<Component>> deserialized_pool = {};
    std::vector<std::shared_ptr<Entity>> deserialized_entities_pool = {};
    std::shared_ptr<Scene> m_scene;

    std::unordered_map<std::string, std::string> m_replaced_guids_map = {};

    DeserializationMode m_deserialization_mode = DeserializationMode::Normal;

    // FIXME: Duplication of paths here and in Editor
    inline static std::string m_prefab_path = "./res/prefabs/";

    inline static std::shared_ptr<SceneSerializer> m_instance;
};
