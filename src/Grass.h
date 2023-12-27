#pragma once

#include "Model.h"

class Grass final : public Model
{
public:
    static std::shared_ptr<Grass> create(std::shared_ptr<Material> const& material, uint32_t const grass_count = 1024);
    static std::shared_ptr<Grass> create(std::shared_ptr<Material> const& material, uint32_t const grass_count = 1024, std::string const& diffuse_texture_path = "");

    explicit Grass(std::shared_ptr<Material> const& material, uint32_t const grass_count);
    explicit Grass(std::shared_ptr<Material> const& material, uint32_t const grass_count, std::string diffuse_texture_path);

    virtual std::string get_name() const override;

    virtual void prepare() override;

private:
    int32_t GRASS_SEGMENTS_HIGH = 6;
    int32_t GRASS_SEGMENTS_LOW = 1;

    float GRASS_TILE_SIZE = 10.0f;

    uint32_t grass_count = 0;
    uint32_t grass_count_x = 0;
    uint32_t grass_count_y = 0;

    std::string diffuse_texture_path;

    [[nodiscard]] Mesh create_blade() const;
};

