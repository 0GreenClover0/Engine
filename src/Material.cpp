#include "Material.h"

#include "Renderer.h"

std::shared_ptr<Material> Material::create(std::shared_ptr<Shader> const& shader, i32 const render_order, bool const is_gpu_instanced,
                                           bool const is_billboard, bool const is_transparent)
{
    auto material =
        std::make_shared<Material>(AK::Badge<Material> {}, shader, render_order, is_gpu_instanced, is_billboard, is_transparent);

    if (is_transparent)
    {
        material->m_render_order = Renderer::transparent_render_order;
    }

    Renderer::get_instance()->register_material(material);
    shader->materials.emplace_back(material);

    return material;
}

Material::Material(AK::Badge<Material>, std::shared_ptr<Shader> const& shader, i32 const render_order, bool const is_gpu_instanced,
                   bool const is_billboard, bool const is_transparent)
    : shader(shader), is_billboard(is_billboard), is_transparent(is_transparent), is_gpu_instanced(is_gpu_instanced),
      m_render_order(render_order)
{
}

i32 Material::get_render_order() const
{
    return m_render_order;
}
