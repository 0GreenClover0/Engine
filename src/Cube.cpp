#include "Cube.h"

#include "Globals.h"

Cube::Cube(std::shared_ptr<Material> const& material) : Model(material)
{
    draw_type = GL_TRIANGLES;
    Cube::prepare();
}

Cube::Cube(std::string diffuse_texture_path, std::shared_ptr<Material> const& material) : Model(material), diffuse_texture_path(std::move(diffuse_texture_path))
{
    draw_type = GL_TRIANGLES;
    Cube::prepare();
}

Cube::Cube(std::string diffuse_texture_path, std::string specular_texture_path, std::shared_ptr<Material> const& material)
    : Model(material), diffuse_texture_path(std::move(diffuse_texture_path)), specular_texture_path(std::move(specular_texture_path))
{
    draw_type = GL_TRIANGLES;
    Cube::prepare();
}

void Cube::prepare()
{
    meshes.emplace_back(create_cube());
}

void Cube::reset()
{
    meshes.clear();
}

void Cube::reprepare()
{
    Cube::reset();
    Cube::prepare();
}

Mesh Cube::create_cube() const
{
    std::vector<Vertex> const vertices = InternalMeshData::cube.vertices;
    std::vector<uint32_t> const indices = InternalMeshData::cube.indices;
    std::vector<Texture> textures;

    std::vector<Texture> diffuse_maps = {};
    if (!diffuse_texture_path.empty())
        diffuse_maps.emplace_back(load_texture(diffuse_texture_path, "texture_diffuse"));

    std::vector<Texture> specular_maps = {};
    if (!specular_texture_path.empty())
        specular_maps.emplace_back(load_texture(specular_texture_path, "texture_specular"));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    return Mesh::create(vertices, indices, textures, draw_type, material);
}

Texture Cube::load_texture(std::string const& path, std::string const& type) const
{
    Texture texture;
    texture.id = texture_from_file(path.c_str());
    texture.type = type;
    texture.path = diffuse_texture_path;
    return texture;
}
