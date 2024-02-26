#include "ShaderGL.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

ShaderGL::ShaderGL(AK::Badge<ShaderFactory>, std::string const& compute_path) : Shader(compute_path)
{
    char const* compute_path_c_str = m_compute_path.c_str();

    program_id = glCreateProgram();

    i32 const compute_shader_id = attach(compute_path_c_str, GL_COMPUTE_SHADER);

    if (compute_shader_id == -1)
        return;

    glLinkProgram(program_id);

    i32 success = 0;
    i32 constexpr log_size = 512;
    char info_log[log_size] = {};

    glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program_id, log_size, nullptr, info_log);
        std::cout << "Error. OpenGL program linking failed" << std::endl << info_log << std::endl;
        return;
    }

    glDeleteShader(compute_shader_id);
}

ShaderGL::ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path)
    : Shader(vertex_path, fragment_path)
{
    char const* vertex_path_c_str = m_vertex_path.c_str();
    char const* fragment_path_c_str = m_fragment_path.c_str();

    program_id = glCreateProgram();

    i32 const vertex_shader_id = attach(vertex_path_c_str, GL_VERTEX_SHADER);

    if (vertex_shader_id == -1)
        return;

    i32 const fragment_shader_id = attach(fragment_path_c_str, GL_FRAGMENT_SHADER);

    if (fragment_shader_id == -1)
        return;

    glLinkProgram(program_id);

    i32 success = 0;
    i32 constexpr log_size = 512;
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

ShaderGL::ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path)
    : Shader(vertex_path, fragment_path, geometry_path)
{
    char const* vertex_path_c_str = m_vertex_path.c_str();
    char const* fragment_path_c_str = m_fragment_path.c_str();
    char const* geometry_path_c_str = m_geometry_path.c_str();

    program_id = glCreateProgram();

    i32 const vertex_shader_id = attach(vertex_path_c_str, GL_VERTEX_SHADER);

    if (vertex_shader_id == -1)
        return;

    i32 const fragment_shader_id = attach(fragment_path_c_str, GL_FRAGMENT_SHADER);

    if (fragment_shader_id == -1)
        return;

    i32 const geometry_shader_id = attach(geometry_path_c_str, GL_GEOMETRY_SHADER);

    if (geometry_shader_id == -1)
        return;

    glLinkProgram(program_id);

    i32 success = 0;
    i32 constexpr log_size = 512;
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

ShaderGL::ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& tessellation_control_path,
                   std::string const& tessellation_evaluation_path,
                   std::string const& fragment_path) : Shader(vertex_path, tessellation_control_path, tessellation_evaluation_path,
                                                              fragment_path)
{
    char const* vertex_path_c_str = m_vertex_path.c_str();
    char const* tessellation_control_path_c_str = m_tessellation_control_path.c_str();
    char const* tessellation_evaluation_path_c_str = m_tessellation_evaluation_path.c_str();
    char const* fragment_path_c_str = m_fragment_path.c_str();

    program_id = glCreateProgram();

    i32 const vertex_shader_id = attach(vertex_path_c_str, GL_VERTEX_SHADER);

    if (vertex_shader_id == -1)
        return;

    i32 const fragment_shader_id = attach(fragment_path_c_str, GL_FRAGMENT_SHADER);

    if (fragment_shader_id == -1)
        return;

    i32 const tessellation_control_shader_id = attach(tessellation_control_path_c_str, GL_TESS_CONTROL_SHADER);

    if (tessellation_control_shader_id == -1)
        return;

    i32 const tessellation_evaluation_shader_id = attach(tessellation_evaluation_path_c_str, GL_TESS_EVALUATION_SHADER);

    if (tessellation_evaluation_shader_id == -1)
        return;

    glLinkProgram(program_id);

    i32 success = 0;
    i32 constexpr log_size = 512;
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
    glDeleteShader(tessellation_control_shader_id);
    glDeleteShader(tessellation_evaluation_shader_id);
}

i32 ShaderGL::attach(char const* path, i32 const type) const
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
        std:: cout << "Error. ShaderGL file not successfully read." << "\n" << e.what() << "\n";
    }

    char const* shader_code = code.c_str();

    i32 success = 0;
    i32 constexpr log_size = 512;
    char info_log[log_size] = {};

    i32 const shader_id = glCreateShader(type);
    glShaderSource(shader_id, 1, &shader_code, nullptr);
    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader_id, log_size, nullptr, info_log);
        std::cout << "Error. ShaderGL of type " << type << ", linking failed." << "\n" << info_log << "\n";
        return -1;
    }

    glAttachShader(program_id, shader_id);
    return shader_id;
}

void ShaderGL::use() const
{
    glUseProgram(program_id);
}

void ShaderGL::set_bool(std::string const& name, bool const value) const
{
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), static_cast<i32>(value));
}

void ShaderGL::set_int(std::string const& name, i32 const value) const
{
    glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
}

void ShaderGL::set_float(std::string const& name, float const value) const
{
    glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
}

void ShaderGL::set_vec3(std::string const& name, glm::vec3 const value) const
{
    glUniform3fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void ShaderGL::set_vec4(std::string const& name, glm::vec4 const value) const
{
    glUniform4fv(glGetUniformLocation(program_id, name.c_str()), 1, glm::value_ptr(value));
}

void ShaderGL::set_mat4(std::string const& name, glm::mat4 const value) const
{
    glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
