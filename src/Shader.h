#pragma once

#include <string>
#include <glm/fwd.hpp>

class Shader
{
public:
    unsigned int program_id = {};

    Shader() = delete;
    Shader(std::string vertex_path, std::string fragment_path);
    Shader(std::string vertex_path, std::string fragment_path, std::string geometry_path);

    void use() const;
    void set_bool(std::string const& name, bool value) const;
    void set_int(std::string const& name, int value) const;
    void set_float(std::string const& name, float value) const;
    void set_vec3(std::string const& name, glm::vec3 value) const;
    void set_vec4(std::string const& name, glm::vec4 value) const;
    void set_mat4(std::string const& name, glm::mat4 value) const;

private:
    int32_t attach(char const* path, int type) const;

    std::string vertex_path = {};
    std::string fragment_path = {};
    std::string geometry_path = {};
};
