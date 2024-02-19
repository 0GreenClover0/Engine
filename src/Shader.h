#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/fwd.hpp>

class Material;

class Shader
{
public:
    unsigned int program_id = {};

    Shader() = delete;
    virtual ~Shader() = default;

    void virtual use() const = 0;
    void virtual set_bool(std::string const& name, bool value) const = 0;
    void virtual set_int(std::string const& name, int value) const = 0;
    void virtual set_float(std::string const& name, float value) const = 0;
    void virtual set_vec3(std::string const& name, glm::vec3 value) const = 0;
    void virtual set_vec4(std::string const& name, glm::vec4 value) const = 0;
    void virtual set_mat4(std::string const& name, glm::mat4 value) const = 0;

    std::vector<std::shared_ptr<Material>> materials;

protected:
    explicit Shader(std::string const& compute_path);
    explicit Shader(std::string const& vertex_path, std::string const& fragment_path);
    explicit Shader(std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path);
    explicit Shader(std::string const& vertex_path, std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
                    std::string const& fragment_path);

    int32_t virtual attach(char const* path, int type) const = 0;

    std::string compute_path = {};
    std::string vertex_path = {};
    std::string tessellation_control_path = {};
    std::string tessellation_evaluation_path = {};
    std::string fragment_path = {};
    std::string geometry_path = {};

private:
    friend class SceneSerializer;
};
