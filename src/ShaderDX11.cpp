#include "ShaderDX11.h"

#include "AK/AK.h"
#include "Renderer.h"
#include "RendererDX11.h"

#include <d3dcommon.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler")

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& compute_path) : Shader(compute_path)
{
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path)
    : Shader(vertex_path, fragment_path)
{
    load_shader();
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& fragment_path,
                       std::string const& geometry_path)
    : Shader(vertex_path, fragment_path, geometry_path)
{
}

ShaderDX11::ShaderDX11(AK::Badge<ShaderFactory>, std::string const& vertex_path, std::string const& tessellation_control_path,
                       std::string const& tessellation_evaluation_path, std::string const& fragment_path)
    : Shader(vertex_path, tessellation_control_path, tessellation_evaluation_path, fragment_path)
{
}

void ShaderDX11::load_shader()
{
    HRESULT hr;
    // Load vertex shader
    ID3DBlob* vs_blob;
    {
        ID3DBlob* shader_compile_errors_blob;

        std::wstring const vertex_path_final = std::wstring(m_vertex_path.begin(), m_vertex_path.end());
        size_t size = 0;
        char const* shader_source = read_hlsl_shader_from_file(m_vertex_path, &size);
        hr = D3DPreprocess(shader_source, size, m_vertex_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, &vs_blob,
                           &shader_compile_errors_blob);

        delete[] shader_source;

        if (FAILED(hr))
        {
            char const* error_string = nullptr;
            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
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

        std::string const hash =
            std::to_string(AK::murmur_hash(static_cast<u8*>(vs_blob->GetBufferPointer()), vs_blob->GetBufferSize(), 0));

        if (!read_file_to_blob(m_vertex_path + hash + "vs_main", &vs_blob))
        {
            hr = D3DCompile(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0,
                            &vs_blob, &shader_compile_errors_blob);

            if (FAILED(hr))
            {
                auto const error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                std::cout << error_string << "\n";
                return;
            }

            save_compiled_shader(m_vertex_path + hash + "vs_main", vs_blob);
        }

        hr = RendererDX11::get_instance_dx11()->get_device()->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(),
                                                                                 nullptr, &m_vertex_shader);
        if (FAILED(hr))
        {
            std::cout << "Error. Vertex shader creation failed."
                      << "\n";
            vs_blob->Release();
            return;
        }
    }

    // Load pixel shader
    ID3DBlob* ps_blob;
    {
        ID3DBlob* shader_compile_errors_blob;

        auto const pixel_path_final = std::wstring(m_fragment_path.begin(), m_fragment_path.end());
        size_t size = 0;
        auto const shader_source = read_hlsl_shader_from_file(m_fragment_path, &size);
        hr = D3DPreprocess(shader_source, size, m_fragment_path.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, &ps_blob,
                           &shader_compile_errors_blob);

        delete[] shader_source;

        if (FAILED(hr))
        {
            char const* error_string = nullptr;

            if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                error_string = "Error. Fragment shader file not found.";
            }
            else if (shader_compile_errors_blob)
            {
                error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                shader_compile_errors_blob->Release();
            }

            std::cout << error_string << "\n";
            return;
        }

        std::string const hash =
            std::to_string(AK::murmur_hash(static_cast<u8*>(ps_blob->GetBufferPointer()), ps_blob->GetBufferSize(), 0));

        if (!read_file_to_blob(m_fragment_path + hash + "ps_main", &ps_blob))
        {
            hr = D3DCompile(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, nullptr, nullptr, "ps_main", "ps_5_0", 0, 0,
                            &ps_blob, &shader_compile_errors_blob);

            if (FAILED(hr))
            {
                auto const error_string = static_cast<char const*>(shader_compile_errors_blob->GetBufferPointer());
                std::cout << error_string << "\n";
                return;
            }

            save_compiled_shader(m_vertex_path + hash + "ps_main", ps_blob);
        }

        hr = RendererDX11::get_instance_dx11()->get_device()->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(),
                                                                                nullptr, &m_pixel_shader);

        if (FAILED(hr))
        {
            std::cout << "Error. Fragment shader creation failed."
                      << "\n";
            ps_blob->Release();
            return;
        }
    }

    {
        std::array<D3D11_INPUT_ELEMENT_DESC, 3> constexpr input_element_desc = {
            {{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
             {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
             {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}}};

        HRESULT const hr = RendererDX11::get_instance_dx11()->get_device()->CreateInputLayout(
            input_element_desc.data(), input_element_desc.size(), vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &m_input_layout);
        assert(SUCCEEDED(hr));
        vs_blob->Release();
    }
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

char* ShaderDX11::read_hlsl_shader_from_file(std::string const& path, size_t* p_size)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        *p_size = 0;
        return nullptr;
    }

    std::streamsize const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    auto const buffer = new char[file_size + 1];

    if (file.read(buffer, file_size))
    {
        buffer[file_size] = '\0'; // Null-terminate the string
        *p_size = static_cast<size_t>(file_size);
        return buffer;
    }

    delete[] buffer;
    *p_size = 0;
    return nullptr;
}

bool ShaderDX11::save_compiled_shader(std::string const& path, ID3DBlob* p_blob)
{
    size_t const last_slash_pos = path.find_last_of('/');
    std::string const filename = path.substr(last_slash_pos + 1);
    std::string const full_path = m_compiled_path + filename;

    std::filesystem::path const directory = m_compiled_path;

    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }

    if (!p_blob)
    {
        std::cerr << "Invalid ID3DBlob pointer." << std::endl;
        return false;
    }

    std::ofstream file(full_path, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open file: " << full_path << std::endl;
        return false;
    }

    file.write(static_cast<char const*>(p_blob->GetBufferPointer()), p_blob->GetBufferSize());

    if (!file)
    {
        std::cerr << "Failed to write to file: " << full_path << std::endl;
        return false;
    }

    file.close();

    return true;
}

bool ShaderDX11::read_file_to_blob(std::string const& path, ID3DBlob** pp_blob)
{
    size_t const last_slash_pos = path.find_last_of('/');
    std::string const filename = path.substr(last_slash_pos + 1);
    std::string const full_path = m_compiled_path + filename;

    std::filesystem::path const directory = m_compiled_path;

    if (!std::filesystem::exists(directory))
    {
        std::filesystem::create_directories(directory);
    }

    if (!pp_blob)
    {
        return false;
    }

    std::ifstream file(full_path, std::ios::binary | std::ios::ate);

    if (!file.is_open())
    {
        return false;
    }

    std::streamsize const file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    HRESULT const hr = D3DCreateBlob(file_size, pp_blob);

    if (FAILED(hr))
    {
        std::cerr << "Failed to create blob." << std::endl;
        return false;
    }

    if (!file.read(static_cast<char*>((*pp_blob)->GetBufferPointer()), file_size))
    {
        std::cerr << "Failed to read file: " << full_path << std::endl;
        (*pp_blob)->Release();
        *pp_blob = nullptr;
        return false;
    }

    return true;
}
