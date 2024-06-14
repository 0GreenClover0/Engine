#pragma once

#include <d3d11.h>

#include "AK/Badge.h"
#include "Shader.h"

class ShaderFactory;

class ShaderDX11 final : public Shader
{
public:
    explicit ShaderDX11(AK::Badge<ShaderFactory>, std::string const& compute_path);
    explicit ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path);
    explicit ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path,
                        std::string const& geometry_path);
    explicit ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& tessellation_control_path,
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

    static char* read_hlsl_shader_from_file(std::string const& path, size_t* p_size);
    static bool save_compiled_shader(std::string const& path, ID3DBlob* blob);
    static bool read_file_to_blob(std::string const& path, ID3DBlob** pp_blob);

    ID3D11InputLayout* m_input_layout = nullptr;
    ID3D11VertexShader* m_vertex_shader = nullptr;
    ID3D11PixelShader* m_pixel_shader = nullptr;

    // NOTE: Do not use constexpr here! The string will not live until runtime because of that.
    //       https://developercommunity.visualstudio.com/t/c20-constexpr-stdstring-with-static-is-not-working/1441363
    inline static std::string m_compiled_path = "./res/shaders/compiled/";
};
