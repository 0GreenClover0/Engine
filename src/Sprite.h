#pragma once

#include "Model.h"
#include "AK/Badge.h"

class Sprite final : public Model
{
public:
    static std::shared_ptr<Sprite> create(std::shared_ptr<Material> const& material);
    static std::shared_ptr<Sprite> create(std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path);

    explicit Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material);
    explicit Sprite(AK::Badge<Sprite>, std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path);

    virtual std::string get_name() const override;

    virtual void prepare() override;

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_sprite() const;

    std::string m_diffuse_texture_path;
};

