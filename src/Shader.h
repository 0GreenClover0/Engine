#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/fwd.hpp>

#include "AK/Types.h"

class Material;

class Shader
{
public:
    u32 program_id = {};

    Shader() = delete;
    virtual ~Shader() = default;

    void virtual use() const = 0;
    void virtual set_bool(std::string const& name, bool const value) const = 0;
    void virtual set_int(std::string const& name, i32 const value) const = 0;
    void virtual set_float(std::string const& name, float const value) const = 0;
    void virtual set_vec3(std::string const& name, glm::vec3 const value) const = 0;
    void virtual set_vec4(std::string const& name, glm::vec4 const value) const = 0;
    void virtual set_mat4(std::string const& name, glm::mat4 const value) const = 0;

    std::vector<std::shared_ptr<Material>> materials;

protected:
    explicit Shader(std::string const& compute_path);
    explicit Shader(std::string const& vertex_path, std::string const& fragment_path);
    explicit Shader(std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path);
    explicit Shader(std::string const& vertex_path, std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
                    std::string const& fragment_path);

    i32 virtual attach(char const* path, i32 type) const = 0;

    std::string m_compute_path = {};
    std::string m_vertex_path = {};
    std::string m_tessellation_control_path = {};
    std::string m_tessellation_evaluation_path = {};
    std::string m_fragment_path = {};
    std::string m_geometry_path = {};

private:
    friend class SceneSerializer;
};
