#pragma once

#include <vector>

#include "Vertex.h"
#include "Texture.h"

inline double delta_time;

struct MeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<Texture> textures;
};

namespace InternalMeshData
{

inline MeshData cube;

void initialize();

}