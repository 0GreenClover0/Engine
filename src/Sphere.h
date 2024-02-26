#pragma once

#include "Model.h"

class Sphere final : public Model
{
public:
    Sphere(float radius, u32 sectors, u32 stacks, std::string texture_path, std::shared_ptr<Material> const& material);

    virtual std::string get_name() const override;

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

    [[nodiscard]] std::shared_ptr<Mesh> create_sphere() const;

    u32 sector_count;
    u32 stack_count;

    inline static bool use_geometry_shader = false;

private:
    [[nodiscard]] Texture load_texture() const;

    std::string m_texture_path;
    float m_radius;

    friend class SceneSerializer;
};
