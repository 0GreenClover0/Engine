#include "Cube.h"
#include "Globals.h"

Cube::Cube(std::string texture_path, std::shared_ptr<Material> const& material) : Model(material), texture_path(std::move(texture_path))
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

	std::vector<Texture> diffuse_maps = { load_texture() };
	textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

	return Mesh::create(vertices, indices, textures, draw_type, material);
}

Texture Cube::load_texture() const
{
	Texture texture;
	texture.id = texture_from_file(texture_path.c_str());
	texture.type = "texture_diffuse";
	texture.path = texture_path;
    return texture;
}
