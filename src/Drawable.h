#pragma once

#include "Component.h"
#include "Material.h"

class Drawable : public Component
{
public:
    explicit Drawable(std::shared_ptr<Material> const& material);
    explicit Drawable(std::shared_ptr<Material> const& material, int32_t const render_order);
    ~Drawable() override = default;
    virtual void initialize() override;
    virtual void draw() const = 0;

    std::shared_ptr<Material> material;
    int32_t render_order = 0;
};
