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
    std::string get_name() const override = 0;

    virtual void draw() const = 0;

    virtual void draw_instanced(int32_t const size);

    std::shared_ptr<Material> material;
    int32_t render_order = 0;
};
