#pragma once

#include <memory>

#include "DrawType.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "Vertex.h"
#include "AK/Types.h"

class ResourceManager;

class MeshFactory
{
public:
    friend class ResourceManager;

private:
    static std::shared_ptr<Mesh> create(std::vector<Vertex> const& vertices, std::vector<u32> const& indices,
                                        std::vector<std::shared_ptr<Texture>> const& textures, DrawType const draw_type,
                                        std::shared_ptr<Material> const& material,
                                        DrawFunctionType const draw_function = DrawFunctionType::Indexed);
};

