#include "ShaderDX11.h"

#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "Renderer.h"
#include "RendererDX11.h"

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& compute_path) : Shader(compute_path)
{
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path)
    : Shader(vertex_path, fragment_path)
{
    ID3DBlob* vs_blob;
    ID3D11VertexShader* vertex_shader;

    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const vertex_path_final = std::wstring(vertex_path.begin(), vertex_path.end());
        HRESULT h_result = D3DCompileFromFile(vertex_path_final.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vs_blob, &shader_compile_errors_blob);

        if (FAILED(h_result))
        {
            char const* error_string = nullptr;
            if (h_result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. ShaderDX file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        h_result = dynamic_pointer_cast<RendererDX11>(Renderer::get_instance())->get_device()->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &vertex_shader);

        // FIXME: Check for result
    }
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path,
                       std::string const& geometry_path) : Shader(vertex_path, fragment_path, geometry_path)
{
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path,
                       std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
                       std::string const& fragment_path)
    : Shader(vertex_path, tessellation_control_path, tessellation_evaluation_path, fragment_path)
{
}

void ShaderDX11::use() const
{
}

void ShaderDX11::set_bool(std::string const& name, bool const value) const
{
}

void ShaderDX11::set_int(std::string const& name, i32 const value) const
{
}

void ShaderDX11::set_float(std::string const& name, float const value) const
{
}

void ShaderDX11::set_vec3(std::string const& name, glm::vec3 const value) const
{
}

void ShaderDX11::set_vec4(std::string const& name, glm::vec4 const value) const
{
}

void ShaderDX11::set_mat4(std::string const& name, glm::mat4 const value) const
{
}

i32 ShaderDX11::attach(char const* path, i32 type) const
{
    return 0;
}
