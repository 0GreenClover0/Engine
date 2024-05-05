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
    virtual void uninitialize() override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void draw() const = 0;

    virtual void draw_instanced(i32 const size);

    virtual void calculate_bounding_box();
    virtual void adjust_bounding_box();
    virtual BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const;

    NON_SERIALIZED
    BoundingBox bounds = {};

    std::shared_ptr<Material> material = nullptr;

private:
    friend class SceneSerializer;
};
