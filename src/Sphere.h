#pragma once

#include "Model.h"

class Sphere final : public Model
{
public:
    Sphere(float radius, uint32_t sectors, uint32_t stacks, std::string texture_path, std::shared_ptr<Material> const& material);

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

    [[nodiscard]] Mesh create_sphere() const;

    uint32_t sector_count;
    uint32_t stack_count;

    inline static bool use_geometry_shader = false;

private:
    [[nodiscard]] Texture load_texture() const;

    std::string texture_path;
    float radius;
};
