#include "Cube.h"

#include <memory>
#include <sstream>

#include "Globals.h"
#include "MeshFactory.h"
#include "ResourceManager.h"

std::shared_ptr<Cube> Cube::create()
{
    auto cube = std::make_shared<Cube>(AK::Badge<Cube> {}, default_material);
    cube->prepare();

    return cube;
}

std::shared_ptr<Cube> Cube::create(std::shared_ptr<Material> const& material, bool const big_cube)
{
    auto cube = std::make_shared<Cube>(AK::Badge<Cube> {}, material);
    cube->m_big_cube = big_cube;
    cube->prepare();

    return cube;
}

std::shared_ptr<Cube> Cube::create(std::string const& diffuse_texture_path, std::shared_ptr<Material> const& material, bool const big_cube)
{
    AK::Badge<Cube> badge;
    auto cube = std::make_shared<Cube>(AK::Badge<Cube> {}, diffuse_texture_path, material);
    cube->m_big_cube = big_cube;
    cube->prepare();

    return cube;
}

std::shared_ptr<Cube> Cube::create(std::string const& diffuse_texture_path, std::string const& specular_texture_path,
                                   std::shared_ptr<Material> const& material, bool const big_cube)
{
    AK::Badge<Cube> badge;
    auto cube = std::make_shared<Cube>(AK::Badge<Cube> {}, diffuse_texture_path, specular_texture_path, material);
    cube->m_big_cube = big_cube;
    cube->prepare();

    return cube;
}

Cube::Cube(AK::Badge<Cube>, std::shared_ptr<Material> const& material) : Model(material)
{
    m_draw_type = DrawType::Triangles;
}

Cube::Cube(AK::Badge<Cube>, std::string const& diffuse_texture_path, std::shared_ptr<Material> const& material)
    : Model(material), diffuse_texture_path(diffuse_texture_path)
{
    m_draw_type = DrawType::Triangles;
}

Cube::Cube(AK::Badge<Cube>, std::string const& diffuse_texture_path, std::string const& specular_texture_path,
           std::shared_ptr<Material> const& material)
    : Model(material), diffuse_texture_path(diffuse_texture_path), specular_texture_path(specular_texture_path)
{
    m_draw_type = DrawType::Triangles;
}

void Cube::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    m_meshes.emplace_back(create_cube());
}

void Cube::reset()
{
    m_meshes.clear();
}

void Cube::reprepare()
{
    Cube::reset();
    Cube::prepare();
}

std::shared_ptr<Mesh> Cube::create_cube() const
{
    std::vector<Vertex> const vertices = m_big_cube ? InternalMeshData::big_cube.vertices : InternalMeshData::cube.vertices;
    std::vector<u32> const indices = m_big_cube ? InternalMeshData::big_cube.indices : InternalMeshData::cube.indices;
    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};
    if (!diffuse_texture_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(diffuse_texture_path, TextureType::Diffuse));

    std::vector<std::shared_ptr<Texture>> specular_maps = {};
    if (!specular_texture_path.empty())
        specular_maps.emplace_back(ResourceManager::get_instance().load_texture(specular_texture_path, TextureType::Specular));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    std::stringstream stream;
    stream << m_big_cube << "CUBE";

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), stream.str(), vertices, indices, textures, m_draw_type, material);
}
