#include "ShaderFactory.h"

#include "Renderer.h"
#include "ShaderGL.h"

std::shared_ptr<Shader> ShaderFactory::create(std::string const& compute_path)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        auto shader = std::make_shared<ShaderGL>(AK::Badge<ShaderFactory> {}, compute_path);

        // Since this is a compute shader we don't need to register it, as we only use registered shaders for drawables

        return shader;
    }

    std::unreachable();
}

std::shared_ptr<Shader> ShaderFactory::create(std::string const& vertex_path, std::string const& fragment_path)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        auto shader = std::make_shared<ShaderGL>(AK::Badge<ShaderFactory> {}, vertex_path, fragment_path);

        Renderer::get_instance()->register_shader(shader);

        return shader;
    }

    std::unreachable();
}

std::shared_ptr<Shader> ShaderFactory::create(std::string const& vertex_path, std::string const& fragment_path,
                                              std::string const& geometry_path)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        auto shader = std::make_shared<ShaderGL>(AK::Badge<ShaderFactory> {}, vertex_path, fragment_path, geometry_path);

        Renderer::get_instance()->register_shader(shader);

        return shader;
    }

    std::unreachable();
}

std::shared_ptr<Shader> ShaderFactory::create(std::string const& vertex_path,
                                              std::string const& tessellation_control_path, std::string const& tessellation_evaluation_path,
                                              std::string const& fragment_path)
{
    if (Renderer::renderer_api == Renderer::RendererApi::OpenGL)
    {
        auto shader = std::make_shared<ShaderGL>(
            AK::Badge<ShaderFactory> {}, vertex_path, tessellation_control_path, tessellation_evaluation_path, fragment_path
        );

        Renderer::get_instance()->register_shader(shader);

        return shader;
    }

    std::unreachable();
}
