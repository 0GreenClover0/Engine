#pragma once

#include <memory>
#include <string>
#include <glm/fwd.hpp>

class Shader
{
public:
    unsigned int program_id = {};

    static std::shared_ptr<Shader> create(std::string compute_path);
    static std::shared_ptr<Shader> create(std::string vertex_path, std::string fragment_path);
    static std::shared_ptr<Shader> create(std::string vertex_path, std::string fragment_path, std::string geometry_path);

    Shader() = delete;

    void use() const;
    void set_bool(std::string const& name, bool value) const;
    void set_int(std::string const& name, int value) const;
    void set_float(std::string const& name, float value) const;
    void set_vec3(std::string const& name, glm::vec3 value) const;
    void set_vec4(std::string const& name, glm::vec4 value) const;
    void set_mat4(std::string const& name, glm::mat4 value) const;

private:
    Shader(std::string compute_path);
    Shader(std::string vertex_path, std::string fragment_path);
    Shader(std::string vertex_path, std::string fragment_path, std::string geometry_path);

    int32_t attach(char const* path, int type) const;

    std::string compute_path = {};
    std::string vertex_path = {};
    std::string fragment_path = {};
    std::string geometry_path = {};

    friend class SceneSerializer;
};
