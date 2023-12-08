#pragma once

#include "Component.h"
#include "MaterialInstance.h"

class Drawable : public Component
{
public:
    explicit Drawable(std::shared_ptr<MaterialInstance> const& material_instance);
    explicit Drawable(std::shared_ptr<MaterialInstance> const& material_instance, int32_t const render_order);
    ~Drawable() override = default;
    virtual void initialize() override;
    virtual void draw() const = 0;

    virtual void draw_instanced(int32_t const size);

    std::shared_ptr<MaterialInstance> material_instance;
    int32_t render_order = 0;
};
