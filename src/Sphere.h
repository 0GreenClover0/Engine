#pragma once

#include "Model.h"

class Sphere final : public Model
{
public:
    static std::shared_ptr<Sphere> create();
    static std::shared_ptr<Sphere> create(float radius, u32 sectors, u32 stacks, std::string const& texture_path, std::shared_ptr<Material> const& material);

    explicit Sphere(AK::Badge<Sphere>, std::shared_ptr<Material> const& material);
    Sphere(AK::Badge<Sphere>, float radius, u32 sectors, u32 stacks, std::string const& texture_path, std::shared_ptr<Material> const& material);

    virtual std::string get_name() const override;

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

    [[nodiscard]] std::shared_ptr<Mesh> create_sphere() const;

    u32 sector_count = 0;
    u32 stack_count = 0;
    std::string texture_path = "";
    float radius = 5.0f;

    inline static bool use_geometry_shader = false;

private:
    [[nodiscard]] Texture load_texture() const;

    friend class SceneSerializer;
};
