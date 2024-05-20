#pragma once

#include "AK/Badge.h"
#include "Shader.h"

class ShaderFactory;

class ShaderGL final : public Shader
{
public:
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& compute_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path,
                      std::string const& geometry_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& tessellation_control_path,
                      std::string const& tessellation_evaluation_path, std::string const& fragment_path);

    void virtual use() const override;
    void virtual set_bool(std::string const& name, bool const value) const override;
    void virtual set_int(std::string const& name, i32 const value) const override;
    void virtual set_float(std::string const& name, float const value) const override;
    void virtual set_vec3(std::string const& name, glm::vec3 const value) const override;
    void virtual set_vec4(std::string const& name, glm::vec4 const value) const override;
    void virtual set_mat4(std::string const& name, glm::mat4 const value) const override;
    void virtual load_shader() override;

private:
    i32 virtual attach(char const* path, i32 type) const override;
};
