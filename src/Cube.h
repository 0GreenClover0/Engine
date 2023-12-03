#pragma once

#include "Model.h"

class Cube final : public Model
{
public:
    explicit Cube(std::shared_ptr<Material> const& material);
    explicit Cube(std::string texture_path, std::shared_ptr<Material> const& material);

    virtual void prepare() override;
    virtual void reset() override;
    virtual void reprepare() override;

    [[nodiscard]] Mesh create_cube() const;

private:
    [[nodiscard]] Texture load_texture() const;

    std::string texture_path;

    friend class SceneSerializer;
};
