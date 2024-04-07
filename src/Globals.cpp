#include "Globals.h"

#include <iostream>

#include "Model.h"
#include "ResourceManager.h"
#include "TextureLoader.h"

namespace InternalMeshData
{

void light_initialize()
{
    default_shader = ResourceManager::get_instance().load_shader("./res/shaders/lit.hlsl", "./res/shaders/lit.hlsl");
    default_material = Material::create(default_shader);
}

void initialize()
{
    cube.vertices = std::vector<Vertex>();
    cube.vertices.reserve(24);

    // TODO: I think something is wrong with texture coordinates, I saw some clipping
    float constexpr cube_vertices[] = {
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // A 0
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,  // B 1
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,  // C 2
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,  // D 3

       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // E 4
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  // F 5
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,  // G 6
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,  // H 7
 
       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  // D 8
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  // A 9
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // E 10
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  // H 11

        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  // B 12
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  // C 13
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  // G 14
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,  // F 15
 
       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  // A 16
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,  // B 17
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,  // F 18
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,  // E 19

        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  // C 20
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,  // D 21
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  // H 22
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  // G 23
    };

    float constexpr big_cube_vertices[] = {
       -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  // A 0
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  100.0f,  0.0f,  // B 1
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  100.0f,  100.0f,  // C 2
       -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  100.0f,  // D 3

       -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  // E 4
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  100.0f,  0.0f,  // F 5
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  100.0f,  100.0f,  // G 6
       -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  100.0f,  // H 7

       -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  // D 8
       -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  100.0f,  0.0f,  // A 9
       -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  100.0f,  100.0f,  // E 10
       -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  100.0f,  // H 11

        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,  // B 12
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  100.0f,  0.0f,  // C 13
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  100.0f,  100.0f,  // G 14
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  100.0f,  // F 15

       -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  // A 16
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  100.0f,  0.0f,  // B 17
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  100.0f,  100.0f,  // F 18
       -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  100.0f,  // E 19

        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  // C 20
       -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  100.0f,  0.0f,  // D 21
       -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  100.0f,  100.0f,  // H 22
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  100.0f,  // G 23
    };

    for (u32 i = 0; i < sizeof(cube_vertices) / sizeof(float); i += 8)
    {
        Vertex vertex = {};
        vertex.position = glm::vec3(cube_vertices[i], cube_vertices[i + 1], cube_vertices[i + 2]);
        vertex.normal = glm::vec3(cube_vertices[i + 3], cube_vertices[i + 4], cube_vertices[i + 5]);
        vertex.texture_coordinates = glm::vec2(cube_vertices[i + 6], cube_vertices[i + 7]);
        cube.vertices.emplace_back(vertex);
    }

    for (u32 i = 0; i < sizeof(big_cube_vertices) / sizeof(float); i += 8)
    {
        Vertex vertex = {};
        vertex.position = glm::vec3(big_cube_vertices[i], big_cube_vertices[i + 1], big_cube_vertices[i + 2]);
        vertex.normal = glm::vec3(big_cube_vertices[i + 3], big_cube_vertices[i + 4], big_cube_vertices[i + 5]);
        vertex.texture_coordinates = glm::vec2(big_cube_vertices[i + 6], big_cube_vertices[i + 7]);
        big_cube.vertices.emplace_back(vertex);
    }

    u32 constexpr cube_indices[] = {
        // Front and back
        0, 3, 2,
        2, 1, 0,
        4, 5, 6,
        6, 7 ,4,

        // Left and right
        11, 8, 9,
        9, 10, 11,
        12, 13, 14,
        14, 15, 12,

        // Bottom and top
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    cube.indices = std::vector<u32>();
    cube.indices.reserve(12);

    big_cube.indices = std::vector<u32>();
    big_cube.indices.reserve(12);

    for (auto const index : cube_indices)
    {
        cube.indices.emplace_back(index);
        big_cube.indices.emplace_back(index);
    }

    // Load white texture
    white_texture = ResourceManager::get_instance().load_texture("./res/textures/white.jpg", TextureType::Diffuse);
}

}
