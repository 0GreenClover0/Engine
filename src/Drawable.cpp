#include "Drawable.h"

#include "Entity.h"
#include "Renderer.h"

Drawable::Drawable(std::shared_ptr<Material> const& material)
{
    this->material = material;
}

Drawable::Drawable(std::shared_ptr<Material> const& material, int32_t const render_order) : render_order(render_order)
{
    this->material = material;
}

void Drawable::calculate_bounding_box()
{
}

void Drawable::adjust_bounding_box()
{
}

void Drawable::initialize()
{
    entity->drawables.emplace_back(this);

    Renderer::get_instance()->register_drawable(std::dynamic_pointer_cast<Drawable>(shared_from_this()));

    calculate_bounding_box();
}

void Drawable::draw_instanced(int32_t const size)
{
}
