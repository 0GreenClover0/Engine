#pragma once

#include "Bounds.h"
#include "Component.h"
#include "Material.h"

class Drawable : public Component
{
public:
    explicit Drawable(std::shared_ptr<Material> const& material);
    ~Drawable() override = default;

    virtual void initialize() override;
    std::string get_name() const override = 0;

    virtual void draw() const = 0;

    virtual void draw_instanced(int32_t const size);

    virtual void calculate_bounding_box();
    virtual void adjust_bounding_box();
    virtual BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    BoundingBox bounds = {};

    std::shared_ptr<Material> material;
};
