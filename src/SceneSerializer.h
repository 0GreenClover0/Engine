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

    void serialize(std::string const& file_path) const;
    bool deserialize(std::string const& file_path) const;

private:
    static void serialize_entity(YAML::Emitter& out, std::shared_ptr<Entity> const& entity);
    [[nodiscard]] std::shared_ptr<Entity> deserialize_entity(YAML::Node const& entity) const;

    std::shared_ptr<Scene> m_scene;
};

