#include "Drawable.h"

#include "Entity.h"
#include "Renderer.h"

Drawable::Drawable(std::shared_ptr<Material> const& material) : material(material)
{
}

void Drawable::calculate_bounding_box()
{
}

void Drawable::adjust_bounding_box()
{
}

BoundingBox Drawable::get_adjusted_bounding_box(glm::mat4 const& model_matrix) const
{
    return {};
}

RasterizerDrawType Drawable::get_rasterizer_draw_type() const
{
    return m_rasterizer_draw_type;
}

void Drawable::set_rasterizer_draw_type(RasterizerDrawType const new_draw_mode)
{
    m_rasterizer_draw_type = new_draw_mode;
}

void Drawable::initialize()
{
    Renderer::get_instance()->register_drawable(std::dynamic_pointer_cast<Drawable>(shared_from_this()));

    calculate_bounding_box();
    adjust_bounding_box();
}

void Drawable::uninitialize()
{
    Renderer::get_instance()->unregister_drawable(std::dynamic_pointer_cast<Drawable>(shared_from_this()));
}

void Drawable::on_enabled()
{
    auto const drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());

    // Drawable might have already been registered in initialize() method
    if (!Renderer::get_instance()->is_drawable_registered(drawable))
    {
        Renderer::get_instance()->register_drawable(drawable);
    }
}

void Drawable::on_disabled()
{
    Renderer::get_instance()->unregister_drawable(std::dynamic_pointer_cast<Drawable>(shared_from_this()));
}

void Drawable::draw_instanced(i32 const size)
{
}
