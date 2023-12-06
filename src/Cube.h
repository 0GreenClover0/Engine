#pragma once

#include "Model.h"

class Cube final : public Model
{
public:
    explicit Cube(std::shared_ptr<MaterialInstance> const& material_instance);
    explicit Cube(std::string diffuse_texture_path, std::shared_ptr<MaterialInstance> const& material_instance);
    explicit Cube(std::string diffuse_texture_path, std::string specular_texture_path, std::shared_ptr<MaterialInstance> const& material_instance);

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

    [[nodiscard]] Mesh create_cube() const;

private:
    [[nodiscard]] Texture load_texture(std::string const& path, std::string const& type) const;

    std::string diffuse_texture_path;
    std::string specular_texture_path;

    friend class SceneSerializer;
};
