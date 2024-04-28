#pragma once

#include <string>

#include "Model.h"
#include "Serialization.h"

NON_SERIALIZED
class Terrain final : public Model
{
public:
    static std::shared_ptr<Terrain> create(std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path = "");

    explicit Terrain(AK::Badge<Terrain>, std::shared_ptr<Material> const& material, bool const use_gpu, std::string const& height_map_path = "");

    virtual void draw() const override;

    virtual void prepare() override;

private:
    [[nodiscard]] std::shared_ptr<Mesh> create_terrain_from_height_map_gpu() const;
    [[nodiscard]] std::shared_ptr<Mesh> create_terrain_from_height_map();

    bool m_use_gpu = true;

    u32 m_strips_count = 0;
    u32 m_vertices_per_strip = 0;

    std::string m_height_map_path;
};
