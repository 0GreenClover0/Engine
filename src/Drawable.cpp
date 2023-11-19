#include "Drawable.h"

#include "Entity.h"

Drawable::Drawable(std::shared_ptr<Material> const& material)
{
    this->material = std::make_shared<Material>(*material);
}

void Drawable::initialize()
{
    entity->drawables.emplace_back(this);
}
