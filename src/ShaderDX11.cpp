#include "ShaderDX11.h"

#include <array>
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

    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const vertex_path_final = std::wstring(vertex_path.begin(), vertex_path.end());
        HRESULT h_result = D3DCompileFromFile(vertex_path_final.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vs_blob, &shader_compile_errors_blob);

        if (FAILED(h_result))
        {
            char const* error_string = nullptr;
            if (h_result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. Vertex shader file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        h_result = RendererDX11::get_instance_dx11()->get_device()->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &m_vertex_shader);

        if (FAILED(h_result))
        {
            std::cout << "Error. Vertex shader creation failed." << "\n";
            vs_blob->Release();
            return;
        }
    }

    ID3DBlob* ps_blob;

    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const pixel_path_final = std::wstring(fragment_path.begin(), fragment_path.end());
        HRESULT h_result = D3DCompileFromFile(pixel_path_final.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &ps_blob, &shader_compile_errors_blob);

        if (FAILED(h_result))
        {
            const char* error_string = nullptr;

            if (h_result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. Fragment shader file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<const char*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        h_result = RendererDX11::get_instance_dx11()->get_device()->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &m_pixel_shader);

        if (FAILED(h_result))
        {
            std::cout << "Error. Fragment shader creation failed." << "\n";
            ps_blob->Release();
            return;
        }
    }

    {
        constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 3> input_element_desc =
        {
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            }
        };

        HRESULT const h_result = RendererDX11::get_instance_dx11()->get_device()->CreateInputLayout(input_element_desc.data(), input_element_desc.size(), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &m_input_layout);
        assert(SUCCEEDED(h_result));
        vs_blob->Release();
    }
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path, ConstantBuffer constant_buffer)
    : Shader(vertex_path, fragment_path, constant_buffer)
{
    ID3DBlob* vs_blob;

    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const vertex_path_final = std::wstring(vertex_path.begin(), vertex_path.end());
        HRESULT h_result = D3DCompileFromFile(vertex_path_final.c_str(), nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vs_blob, &shader_compile_errors_blob);

        if (FAILED(h_result))
        {
            char const* error_string = nullptr;
            if (h_result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. Vertex shader file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        h_result = RendererDX11::get_instance_dx11()->get_device()->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &m_vertex_shader);

        if (FAILED(h_result))
        {
            std::cout << "Error. Vertex shader creation failed." << "\n";
            vs_blob->Release();
            return;
        }
    }

    ID3DBlob* ps_blob;

    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const pixel_path_final = std::wstring(fragment_path.begin(), fragment_path.end());
        HRESULT h_result = D3DCompileFromFile(pixel_path_final.c_str(), nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &ps_blob, &shader_compile_errors_blob);

        if (FAILED(h_result))
        {
            const char* error_string = nullptr;

            if (h_result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. Fragment shader file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<const char*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        h_result = RendererDX11::get_instance_dx11()->get_device()->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &m_pixel_shader);

        if (FAILED(h_result))
        {
            std::cout << "Error. Fragment shader creation failed." << "\n";
            ps_blob->Release();
            return;
        }
    }

    {
        constexpr std::array<D3D11_INPUT_ELEMENT_DESC, 3> input_element_desc =
        {
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            }
        };

        HRESULT const h_result = RendererDX11::get_instance_dx11()->get_device()->CreateInputLayout(input_element_desc.data(), input_element_desc.size(), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &m_input_layout);
        assert(SUCCEEDED(h_result));
        vs_blob->Release();
    }

    D3D11_BUFFER_DESC desc;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.ByteWidth = static_cast<UINT>(sizeof(constant_buffer) + (16 - (sizeof(constant_buffer) % 16)));
    desc.StructureByteStride = 0;

    HRESULT h_result = RendererDX11::get_instance_dx11()->get_device()->CreateBuffer(&desc, 0, &m_constant_buffer);
    if (FAILED(h_result))
    {
        const char* error_string = nullptr;
        error_string = "Error. Constant Buffer failed to initialize.";

        std::cout << error_string << "\n";
        return;
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
    auto const instance = RendererDX11::get_instance_dx11();
    instance->get_device_context()->IASetInputLayout(m_input_layout);
    instance->get_device_context()->VSSetShader(m_vertex_shader, nullptr, 0);
    instance->get_device_context()->PSSetShader(m_pixel_shader, nullptr, 0);
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
