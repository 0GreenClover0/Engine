#pragma once

#include "ConstantBufferTypes.h"
#include "ResourceManager.h"

#include "Collider2D.h"
#include "type_traits"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <xstring>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/node/node.h>

#include "FloatersManager.h"
#include "SceneSerializer.h"
#include <Game/Factory.h>
#include <Game/ShipSpawner.h>

template<typename E>
constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

namespace YAML
{
template<>
struct convert<glm::vec2>
{
    static Node encode(glm::vec2 const& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        return node;
    }

    static bool decode(Node const& node, glm::vec2& rhs)
    {
        if (!node.IsSequence() || node.size() != 2)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        return true;
    }
};

template<>
struct convert<glm::vec3>
{
    static Node encode(glm::vec3 const& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        return node;
    }

    static bool decode(Node const& node, glm::vec3& rhs)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        return true;
    }
};

template<>
struct convert<glm::vec4>
{
    static Node encode(glm::vec4 const& rhs)
    {
        Node node;
        node.push_back(rhs.x);
        node.push_back(rhs.y);
        node.push_back(rhs.z);
        node.push_back(rhs.w);
        return node;
    }

    static bool decode(Node const& node, glm::vec4& rhs)
    {
        if (!node.IsSequence() || node.size() != 4)
            return false;

        rhs.x = node[0].as<float>();
        rhs.y = node[1].as<float>();
        rhs.z = node[2].as<float>();
        rhs.w = node[3].as<float>();
        return true;
    }
};

template<>
struct convert<std::wstring>
{
    static Node encode(std::wstring const& rhs)
    {
        std::string const narrow(rhs.begin(), rhs.end());
        return Node(narrow);
    }

    static bool decode(Node const& node, std::wstring& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }

        std::string const& narrow = node.as<std::string>();
        rhs = std::wstring(narrow.begin(), narrow.end());
        return true;
    }
};

template<>
struct convert<std::shared_ptr<Shader>>
{
    static Node encode(std::shared_ptr<Shader> const& rhs)
    {
        Node node;
        node.push_back(rhs->get_vertex_path());
        node.push_back(rhs->get_fragment_path());
        node.push_back(rhs->get_geometry_path());
        return node;
    }

    static bool decode(Node const& node, std::shared_ptr<Shader>& rhs)
    {
        if (node.size() != 3)
            return false;

        auto const vertex_path = node["VertexPath"].as<std::string>();
        auto const fragment_path = node["FragmentPath"].as<std::string>();
        auto const geometry_path = node["GeometryPath"].as<std::string>();

        if (geometry_path.empty())
        {
            rhs = ResourceManager::get_instance().load_shader(vertex_path, fragment_path);
        }
        else
        {
            rhs = ResourceManager::get_instance().load_shader(vertex_path, fragment_path, geometry_path);
        }

        return true;
    }
};

template<>
struct convert<std::shared_ptr<Material>>
{
    static Node encode(std::shared_ptr<Material> const& rhs)
    {
        Node node;
        node.push_back(rhs->shader);
        node.push_back(rhs->color);
        node.push_back(rhs->get_render_order());
        node.push_back(rhs->needs_forward_rendering);
        node.push_back(rhs->casts_shadows);
        node.push_back(rhs->is_billboard);
        return node;
    }

    static bool decode(Node const& node, std::shared_ptr<Material>& rhs)
    {
        if (node.size() != 5 && node.size() != 6)
            return false;

        auto const shader = node["Shader"].as<std::shared_ptr<Shader>>();
        auto const color = node["Color"].as<glm::vec4>();
        auto const render_order = node["RenderOrder"].as<i32>();
        auto const forward_rendered = node["NeedsForward"].as<bool>();
        auto const casts_shadows = node["CastsShadows"].as<bool>();

        bool is_billboard = false;
        if (node["IsBillboard"].IsDefined())
        {
            is_billboard = node["IsBillboard"].as<bool>();
        }

        rhs = Material::create(shader, render_order);
        rhs->color = color;
        rhs->needs_forward_rendering = forward_rendered;
        rhs->casts_shadows = casts_shadows;
        rhs->is_billboard = is_billboard;

        return true;
    }
};

template<typename T>
requires std::is_base_of_v<Component, T> struct convert<std::shared_ptr<T>>
{
    static Node encode(std::shared_ptr<T> const& rhs)
    {
        Node node;
        node.push_back(rhs->guid);
        return node;
    }

    static bool decode(Node const& node, std::shared_ptr<T>& rhs)
    {
        if (node.size() != 1)
            return false;

        rhs = std::dynamic_pointer_cast<T>(SceneSerializer::get_instance()->get_from_pool(node["guid"].as<std::string>()));

        return true;
    }
};

template<typename T>
requires std::is_base_of_v<Component, T> struct convert<std::weak_ptr<T>>
{
    static Node encode(std::weak_ptr<T> const& rhs)
    {
        Node node;

        if (!rhs.expired())
        {
            node.push_back(rhs.lock()->guid);
        }
        else
        {
            node.push_back("nullptr");
        }
        return node;
    }

    static bool decode(Node const& node, std::weak_ptr<T>& rhs)
    {
        if (node.size() != 1)
            return false;

        if (node["guid"].as<std::string>() == "nullptr")
        {
            return true;
        }

        rhs = std::dynamic_pointer_cast<T>(SceneSerializer::get_instance()->get_from_pool(node["guid"].as<std::string>()));

        return true;
    }
};

template<>
struct convert<DXWave>
{
    static Node encode(DXWave const& rhs)
    {
        Node node;
        node.push_back(rhs.direction);
        node.push_back(rhs.padding);
        node.push_back(rhs.speed);
        node.push_back(rhs.steepness);
        node.push_back(rhs.wave_length);
        node.push_back(rhs.amplitude);
        return node;
    }

    static bool decode(Node const& node, DXWave& rhs)
    {
        if (!node.IsSequence() || node.size() != 6)
            return false;

        rhs.direction = node[0].as<glm::vec2>();
        rhs.padding = node[1].as<glm::vec2>();
        rhs.speed = node[2].as<float>();
        rhs.steepness = node[3].as<float>();
        rhs.wave_length = node[4].as<float>();
        rhs.amplitude = node[5].as<float>();
        return true;
    }
};

template<>
struct convert<ColliderType2D>
{
    static Node encode(ColliderType2D const rhs)
    {
        Node node;
        node.push_back(to_integral(rhs));
        return node;
    }

    static bool decode(Node const& node, ColliderType2D& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }

        rhs = static_cast<ColliderType2D>(node.as<int>());
        return true;
    }
};

template<>
struct convert<ConstantBufferWater>
{
    static Node encode(ConstantBufferWater const& rhs)
    {
        Node node;
        node.push_back(rhs.top_color);
        node.push_back(rhs.bottom_color);
        node.push_back(rhs.normalmap_scroll_speed_0);
        node.push_back(rhs.normalmap_scroll_speed_1);
        node.push_back(rhs.normalmap_scale0);
        node.push_back(rhs.normalmap_scale1);
        node.push_back(rhs.combined_amplitude);
        node.push_back(rhs.phong_contribution);
        return node;
    }

    static bool decode(Node const& node, ConstantBufferWater& rhs)
    {
        if (!node.IsSequence() || node.size() != 8)
            return false;

        rhs.top_color = node[0].as<glm::vec4>();
        rhs.bottom_color = node[1].as<glm::vec4>();
        rhs.normalmap_scroll_speed_0 = node[2].as<float>();
        rhs.normalmap_scroll_speed_1 = node[3].as<float>();
        rhs.normalmap_scale0 = node[4].as<float>();
        rhs.normalmap_scale1 = node[5].as<float>();
        rhs.combined_amplitude = node[6].as<float>();
        rhs.phong_contribution = node[7].as<float>();
        return true;
    }
};

template<>
struct convert<FactoryType>
{
    static Node encode(FactoryType const rhs)
    {
        Node node;
        node.push_back(to_integral(rhs));
        return node;
    }

    static bool decode(Node const& node, FactoryType& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }

        rhs = static_cast<FactoryType>(node.as<int>());
        return true;
    }
};

template<>
struct convert<SpawnType>
{
    static Node encode(SpawnType const rhs)
    {
        Node node;
        node.push_back(to_integral(rhs));
        return node;
    }

    static bool decode(Node const& node, SpawnType& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }

        rhs = static_cast<SpawnType>(node.as<int>());
        return true;
    }
};

template<>
struct convert<ShipType>
{
    static Node encode(ShipType const rhs)
    {
        Node node;
        node.push_back(to_integral(rhs));
        return node;
    }

    static bool decode(Node const& node, ShipType& rhs)
    {
        if (!node.IsScalar())
        {
            return false;
        }

        rhs = static_cast<ShipType>(node.as<int>());
        return true;
    }
};

template<>
struct convert<SpawnEvent>
{
    static Node encode(SpawnEvent const& rhs)
    {
        Node node;
        node.push_back(rhs.spawn_list);
        node.push_back(rhs.spawn_type);
        return node;
    }

    static bool decode(Node const& node, SpawnEvent& rhs)
    {
        if (!node.IsSequence())
            return false;

        rhs.spawn_list = node[0].as<std::vector<ShipType>>();
        rhs.spawn_type = node[1].as<SpawnType>();
        return true;
    }
};

template<>
struct convert<FloaterSettings>
{
    static Node encode(FloaterSettings const& rhs)
    {
        Node node;
        node.push_back(rhs.sink_rate);
        node.push_back(rhs.side_rotation_strength);
        node.push_back(rhs.forward_rotation_strength);
        node.push_back(rhs.side_floaters_offset);
        node.push_back(rhs.forward_floaters_offset);
        return node;
    }

    static bool decode(Node const& node, FloaterSettings& rhs)
    {
        if (!node.IsSequence())
            return false;

        rhs.sink_rate = node[0].as<float>();
        rhs.side_rotation_strength = node[1].as<float>();
        rhs.forward_rotation_strength = node[2].as<float>();
        rhs.side_floaters_offset = node[3].as<float>();
        rhs.forward_floaters_offset = node[4].as<float>();

        return true;
    }
};

template<class T>
Emitter& operator<<(YAML::Emitter& out, std::shared_ptr<T> const& v)
requires(std::is_base_of_v<Component, T>)
{
    out << YAML::BeginMap;

    if (v == nullptr)
        out << YAML::Key << "guid" << YAML::Value << "nullptr";
    else
        out << YAML::Key << "guid" << YAML::Value << v->guid;

    out << YAML::EndMap;

    return out;
}

template<class T>
Emitter& operator<<(YAML::Emitter& out, std::weak_ptr<T> const& v)
requires(std::is_base_of_v<Component, T>)
{
    out << YAML::BeginMap;

    if (v.expired())
        out << YAML::Key << "guid" << YAML::Value << "nullptr";
    else
        out << YAML::Key << "guid" << YAML::Value << v.lock()->guid;

    out << YAML::EndMap;

    return out;
}

template<typename T>
requires std::is_base_of_v<Entity, T> struct convert<std::shared_ptr<T>>
{
    static Node encode(std::shared_ptr<T> const& rhs)
    {
        Node node;
        node.push_back(rhs->guid);
        return node;
    }

    static bool decode(Node const& node, std::shared_ptr<T>& rhs)
    {
        if (node.size() != 1)
            return false;

        rhs = std::dynamic_pointer_cast<T>(SceneSerializer::get_instance()->get_entity_from_pool(node["guid"].as<std::string>()));

        return true;
    }
};

template<typename T>
requires std::is_base_of_v<Entity, T> struct convert<std::weak_ptr<T>>
{
    static Node encode(std::weak_ptr<T> const& rhs)
    {
        Node node;

        if (!rhs.expired())
        {
            node.push_back(rhs.lock()->guid);
        }
        else
        {
            node.push_back("nullptr");
        }
        return node;
    }

    static bool decode(Node const& node, std::weak_ptr<T>& rhs)
    {
        if (node.size() != 1)
            return false;

        if (node["guid"].as<std::string>() == "nullptr")
        {
            return true;
        }

        rhs = std::dynamic_pointer_cast<T>(SceneSerializer::get_instance()->get_entity_from_pool(node["guid"].as<std::string>()));

        return true;
    }
};

template<class T>
Emitter& operator<<(YAML::Emitter& out, std::shared_ptr<T> const& v)
requires(std::is_base_of_v<Entity, T>)
{
    out << YAML::BeginMap;

    if (v == nullptr)
        out << YAML::Key << "guid" << YAML::Value << "nullptr";
    else
        out << YAML::Key << "guid" << YAML::Value << v->guid;

    out << YAML::EndMap;

    return out;
}

template<class T>
Emitter& operator<<(YAML::Emitter& out, std::weak_ptr<T> const& v)
requires(std::is_base_of_v<Entity, T>)
{
    out << YAML::BeginMap;

    if (v.expired())
        out << YAML::Key << "guid" << YAML::Value << "nullptr";
    else
        out << YAML::Key << "guid" << YAML::Value << v.lock()->guid;

    out << YAML::EndMap;

    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, glm::vec2 const& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, glm::vec3 const& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, glm::vec4 const& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, std::wstring const& v)
{
    std::string const narrow(v.begin(), v.end());
    out << narrow;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, std::shared_ptr<Shader> const& shader)
{
    out << YAML::BeginMap; // Shader

    out << YAML::Key << "VertexPath" << YAML::Value << shader->get_vertex_path();
    out << YAML::Key << "FragmentPath" << YAML::Value << shader->get_fragment_path();
    out << YAML::Key << "GeometryPath" << YAML::Value << shader->get_geometry_path();

    out << YAML::EndMap; // Shader

    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, std::shared_ptr<Material> const& material)
{
    out << YAML::BeginMap; // Material

    out << YAML::Key << "Shader" << YAML::Value << material->shader;
    out << YAML::Key << "Color" << YAML::Value << material->color;
    out << YAML::Key << "RenderOrder" << YAML::Value << material->get_render_order();
    out << YAML::Key << "NeedsForward" << YAML::Value << material->needs_forward_rendering;
    out << YAML::Key << "CastsShadows" << YAML::Value << material->casts_shadows;
    out << YAML::Key << "IsBillboard" << YAML::Value << material->is_billboard;

    out << YAML::EndMap; // Material

    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, DXWave const& wave)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << wave.direction << wave.padding << wave.speed << wave.steepness << wave.wave_length << wave.amplitude
        << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, ColliderType2D const& v)
{
    out << YAML::Flow;
    out << to_integral(v);
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, FactoryType const& v)
{
    out << YAML::Flow;
    out << to_integral(v);
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, ConstantBufferWater const& cb)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << cb.top_color << cb.bottom_color << cb.normalmap_scroll_speed_0 << cb.normalmap_scroll_speed_1
        << cb.normalmap_scale0 << cb.normalmap_scale1 << cb.combined_amplitude << cb.phong_contribution << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, SpawnType const& v)
{
    out << YAML::Flow;
    out << to_integral(v);
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, ShipType const& v)
{
    out << YAML::Flow;
    out << to_integral(v);
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, SpawnEvent const& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.spawn_list << v.spawn_type << YAML::EndSeq;
    return out;
}

inline Emitter& operator<<(YAML::Emitter& out, FloaterSettings const& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.sink_rate << v.side_rotation_strength << v.forward_rotation_strength << v.side_floaters_offset
        << v.forward_floaters_offset << YAML::EndSeq;
    return out;
}

}
