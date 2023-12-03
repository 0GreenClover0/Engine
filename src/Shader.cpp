#include "Shader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <utility>

#include "Renderer.h"

std::shared_ptr<Shader> Shader::create(std::string vertex_path, std::string fragment_path)
{
    auto shader = std::shared_ptr<Shader>(new Shader(std::move(vertex_path), std::move(fragment_path)));

    Renderer::get_instance()->register_shader(shader);

    return shader;
}


std::shared_ptr<Shader> Shader::create(std::string vertex_path, std::string fragment_path, std::string geometry_path)
{
    auto shader = std::shared_ptr<Shader>(new Shader(std::move(vertex_path), std::move(fragment_path), std::move(geometry_path)));

    Renderer::get_instance()->register_shader(shader);

    return shader;
}

Shader::Shader(std::string vertex_path, std::string fragment_path) : vertex_path(std::move(vertex_path)), fragment_path(std::move(fragment_path))
{
    char const* vertex_path_c_str = this->vertex_path.c_str();
    char const* fragment_path_c_str = this->fragment_path.c_str();

    program_id = glCreateProgram();

    int32_t const vertex_shader_id = attach(vertex_path_c_str, GL_VERTEX_SHADER);

    if (vertex_shader_id == -1)
        return;

    int32_t const fragment_shader_id = attach(fragment_path_c_str, GL_FRAGMENT_SHADER);

    if (fragment_shader_id == -1)
        return;

    glLinkProgram(program_id);

    int32_t success = 0;
    int32_t constexpr log_size = 512;
    char info_log[log_size] = {};

    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, log_size, nullptr, info_log);
        std::cout << "Error. OpenGL program linking failed" << std::endl << info_log << std::endl;
        return;
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
}

Shader::Shader(std::string vertex_path, std::string fragment_path, std::string geometry_path)
    : vertex_path(std::move(vertex_path)), fragment_path(std::move(fragment_path)), geometry_path(std::move(geometry_path))
{
    char const* vertex_path_c_str = this->vertex_path.c_str();
    char const* fragment_path_c_str = this->fragment_path.c_str();
    char const* geometry_path_c_str = this->geometry_path.c_str();

    program_id = glCreateProgram();

    int32_t const vertex_shader_id = attach(vertex_path_c_str, GL_VERTEX_SHADER);

    if (vertex_shader_id == -1)
        return;

    int32_t const fragment_shader_id = attach(fragment_path_c_str, GL_FRAGMENT_SHADER);

    if (fragment_shader_id == -1)
        return;

    int32_t const geometry_shader_id = attach(geometry_path_c_str, GL_GEOMETRY_SHADER);

    if (geometry_shader_id == -1)
        return;

    glLinkProgram(program_id);

    int32_t success = 0;
    int32_t constexpr log_size = 512;
    char info_log[log_size] = {};

    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, log_size, nullptr, info_log);
        std::cout << "Error. OpenGL program linking failed" << std::endl << info_log << std::endl;
        return;
    }

    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    glDeleteShader(geometry_shader_id);
}

int32_t Shader::attach(char const* path, int const type) const
{
    std::string code;
    std::ifstream shader_file;

    shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shader_file.open(path);

        std::stringstream stream;

        stream << shader_file.rdbuf();

        shader_file.close();

        code = stream.str();
    }
    catch (std::ifstream::failure& e)
    {
        std:: cout << "Error. Shader file not successfully read." << "\n" << e.what() << "\n";
    }

    char const* shader_code = code.c_str();

    int32_t success = 0;
    int32_t constexpr log_size = 512;
    char info_log[log_size] = {};

    int32_t const shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &shader_code, nullptr);
    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, log_size, nullptr, info_log);
        std::cout << "Error. Shader of type " << type << ", linking failed." << "\n" << info_log << "\n";
        return -1;
    }

    glAttachShader(program_id, shader_id);
    return shader_id;
}

void Shader::use() const
{
    glUseProgram(program_id);
}

void Shader::set_bool(std::string const& name, bool value) const
{
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), static_cast<int>(value));
}

void Shader::set_int(std::string const& name, int value) const
{
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
}

void Shader::set_float(std::string const& name, float value) const
{
    glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
}

void Shader::set_vec3(std::string const& name, glm::vec3 value) const
{
    glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_vec4(std::string const& name, glm::vec4 value) const
{
    glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::set_mat4(std::string const& name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
