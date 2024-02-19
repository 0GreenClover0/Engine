#include "Shader.h"

Shader::Shader(std::string const& compute_path) : compute_path(compute_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& fragment_path) : vertex_path(vertex_path),
    fragment_path(fragment_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path)
    : vertex_path(vertex_path), fragment_path(fragment_path), geometry_path(geometry_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
               std::string const& fragment_path) : vertex_path(vertex_path),
                                                   tessellation_control_path(tessellation_control_path),
                                                   tessellation_evaluation_path(tessellation_evaluation_path),
                                                   fragment_path(fragment_path)
{
}
