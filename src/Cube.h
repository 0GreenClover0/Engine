#pragma once

#include "Model.h"

class Cube final : public Model
{
public:
    static std::shared_ptr<Cube> create(std::shared_ptr<Material> const& material, bool big_cube = false);
    static std::shared_ptr<Cube> create(std::string const& diffuse_texture_path, std::shared_ptr<Material> const& material, bool big_cube = false);
    static std::shared_ptr<Cube> create(std::string const& diffuse_texture_path, std::string const& specular_texture_path, std::shared_ptr<Material> const& material, bool big_cube = false);

    explicit Cube(std::shared_ptr<Material> const& material);
    explicit Cube(std::string diffuse_texture_path, std::shared_ptr<Material> const& material);
    explicit Cube(std::string diffuse_texture_path, std::string specular_texture_path, std::shared_ptr<Material> const& material);

    std::string get_name() const override;

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

private:
    [[nodiscard]] Mesh create_cube() const;

    std::string diffuse_texture_path;
    std::string specular_texture_path;

    bool big_cube;

    friend class SceneSerializer;
};
