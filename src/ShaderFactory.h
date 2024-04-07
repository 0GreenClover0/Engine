#pragma once

#include <memory>
#include <string>

#include "Shader.h"

class ResourceManager;

class ShaderFactory
{
public:
    ShaderFactory() = delete;

private:
    static std::shared_ptr<Shader> create(std::string const& compute_path);
    static std::shared_ptr<Shader> create(std::string const& vertex_path, std::string const& fragment_path);
    static std::shared_ptr<Shader> create(std::string const& vertex_path, std::string const& fragment_path,
                                          std::string const& geometry_path);
    static std::shared_ptr<Shader> create(std::string const& vertex_path, std::string const& tessellation_control_path,
                                          std::string const& tessellation_evaluation_path, std::string const& fragment_path);

    friend class ResourceManager;
};
