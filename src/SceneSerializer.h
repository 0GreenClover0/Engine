#pragma once

#include <string>
#include <yaml-cpp/node/node.h>

#include "Material.h"
#include "Scene.h"

namespace YAML
{
    class Emitter;
}

class SceneSerializer
{
public:
    explicit SceneSerializer(std::shared_ptr<Scene> const& scene);

    static std::shared_ptr<SceneSerializer> get_instance();
    static void set_instance(std::shared_ptr<SceneSerializer> const& instance);

    [[nodiscard]] std::shared_ptr<Component> get_from_pool(std::string const& guid) const;
    [[nodiscard]] std::shared_ptr<Entity> get_entity_from_pool(std::string const& guid) const;

    void serialize(std::string const& file_path) const;
    bool deserialize(std::string const& file_path);

private:
    static void serialize_entity(YAML::Emitter& out, std::shared_ptr<Entity> const& entity);
    static void auto_serialize_component(YAML::Emitter& out, std::shared_ptr<Component> const& component);
    void auto_deserialize_component(YAML::Node const& component, std::shared_ptr<Entity> const& deserialized_entity, bool const first_pass);

    void deserialize_components(YAML::Node const& entity_node, std::shared_ptr<Entity> const& deserialized_entity, bool const first_pass);

    [[nodiscard]] std::shared_ptr<Entity> deserialize_entity_first_pass(YAML::Node const& entity);
    void deserialize_entity_second_pass(YAML::Node const& entity, std::shared_ptr<Entity> const& deserialized_entity);

    std::vector<std::shared_ptr<Component>> deserialized_pool = {};
    std::vector<std::shared_ptr<Entity>> deserialized_entities_pool = {};
    std::shared_ptr<Scene> m_scene;

    inline static std::shared_ptr<SceneSerializer> m_instance;
};
