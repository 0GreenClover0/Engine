#include "Drawable.h"

#include "Entity.h"
#include "Renderer.h"

Drawable::Drawable(std::shared_ptr<Material> const& material)
{
    m_material = material;
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

void Drawable::initialize()
{
    entity->drawables.emplace_back(this);

    Renderer::get_instance()->register_drawable(std::dynamic_pointer_cast<Drawable>(shared_from_this()));

    calculate_bounding_box();
    adjust_bounding_box();
}

std::shared_ptr<Material> Drawable::material()
{
    return m_material;
}

void Drawable::draw_instanced(int32_t const size)
{
}
