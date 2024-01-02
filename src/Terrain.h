#pragma once

#include <string>

#include "Model.h"

class Terrain final : public Model
{
public:
    static std::shared_ptr<Terrain> create(std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path = "");

    explicit Terrain(std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path = "");

    virtual std::string get_name() const override;

    virtual void draw() const override;

    virtual void prepare() override;

private:
    [[nodiscard]] Mesh create_terrain_from_height_map_gpu() const;
    [[nodiscard]] Mesh create_terrain_from_height_map();

    bool use_gpu = true;

    uint32_t strips_count = 0;
    uint32_t vertices_per_strip = 0;

    std::string height_map_path;
};
