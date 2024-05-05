#pragma once

#include "Model.h"
#include "AK/Badge.h"

NON_SERIALIZED
class Grass final : public Model
{
public:
    static std::shared_ptr<Grass> create();
    static std::shared_ptr<Grass> create(std::shared_ptr<Material> const& material);
    static std::shared_ptr<Grass> create(std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path = "");

    explicit Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material);
    explicit Grass(AK::Badge<Grass>, std::shared_ptr<Material> const& material, std::string const& diffuse_texture_path);

    virtual void prepare() override;

private:
    std::string m_diffuse_texture_path;

    [[nodiscard]] std::shared_ptr<Mesh> create_blade() const;
};

