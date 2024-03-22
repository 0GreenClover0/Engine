#include "Shader.h"

Shader::Shader(std::string const& compute_path) : m_compute_path(compute_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& fragment_path) : m_vertex_path(vertex_path),
m_fragment_path(fragment_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& fragment_path, ConstantBuffer constant_buffer) : m_vertex_path(vertex_path),
    m_fragment_path(fragment_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path)
    : m_vertex_path(vertex_path), m_fragment_path(fragment_path), m_geometry_path(geometry_path)
{
}

Shader::Shader(std::string const& vertex_path, std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
               std::string const& fragment_path) : m_vertex_path(vertex_path),
                                                   m_tessellation_control_path(tessellation_control_path),
                                                   m_tessellation_evaluation_path(tessellation_evaluation_path),
                                                   m_fragment_path(fragment_path)
{
}
