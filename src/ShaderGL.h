#pragma once

#include "Shader.h"
#include "AK/Badge.h"

class ShaderFactory;

class ShaderGL final : public Shader
{
public:
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& compute_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path, std::string const& geometry_path);
    explicit ShaderGL(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
                      std::string const& fragment_path);

    void virtual use() const override;
    void virtual set_bool(std::string const& name, bool value) const override;
    void virtual set_int(std::string const& name, int value) const override;
    void virtual set_float(std::string const& name, float value) const override;
    void virtual set_vec3(std::string const& name, glm::vec3 value) const override;
    void virtual set_vec4(std::string const& name, glm::vec4 value) const override;
    void virtual set_mat4(std::string const& name, glm::mat4 value) const override;

private:
    int32_t virtual attach(char const* path, int type) const override;
};
