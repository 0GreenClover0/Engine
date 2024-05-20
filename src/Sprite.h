#pragma once

#include "AK/Badge.h"
#include "Model.h"

class Sprite final : public Model
{
public:
    static std::shared_ptr<Sprite> create();
    static std::shared_ptr<Sprite> create(std::shared_ptr<Material> const& material);
    static std::shared_ptr<Sprite> create(std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path);

    explicit Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material);
    explicit Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path);

    virtual void prepare() override;

    std::string diffuse_texture_path = "";

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;
};
