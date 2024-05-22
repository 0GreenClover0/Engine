#pragma once

#include "ConstantBufferTypes.h"
#include "ResourceManager.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <xstring>
#include <yaml-cpp/emitter.h>
#include <yaml-cpp/node/node.h>

#include "SceneSerializer.h"

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
        return node;
    }

    static bool decode(Node const& node, std::shared_ptr<Material>& rhs)
    {
        if (node.size() != 5)
            return false;

        auto const shader = node["Shader"].as<std::shared_ptr<Shader>>();
        auto const color = node["Color"].as<glm::vec4>();
        auto const render_order = node["RenderOrder"].as<i32>();
        auto const forward_rendered = node["NeedsForward"].as<bool>();
        auto const casts_shadows = node["CastsShadows"].as<bool>();

        rhs = Material::create(shader, render_order);
        rhs->color = color;
        rhs->needs_forward_rendering = forward_rendered;
        rhs->casts_shadows = casts_shadows;
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
}
