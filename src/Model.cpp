#include "Model.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>

#include "Entity.h"
#include "Mesh.h"
#include "MeshFactory.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Vertex.h"
#include "AK/Types.h"

std::shared_ptr<Model> Model::create(std::string const& model_path, std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(AK::Badge<Model> {}, model_path, material);
    model->prepare();

    return model;
}

std::shared_ptr<Model> Model::create(std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(AK::Badge<Model> {}, material);

    return model;
}

std::shared_ptr<Model> Model::create(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(AK::Badge<Model> {}, material);

    model->m_meshes.emplace_back(mesh);

    return model;
}

Model::Model(AK::Badge<Model>, std::string const& model_path, std::shared_ptr<Material> const& material)
    : Drawable(material), m_model_path(model_path)
{
}

Model::Model(AK::Badge<Model>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Model::calculate_bounding_box()
{
    for (auto const& mesh : m_meshes)
        mesh->calculate_bounding_box();

    // TODO: Merge bounding boxes together
    if (!m_meshes.empty())
        bounds = m_meshes[0]->bounds;
}

void Model::adjust_bounding_box()
{
    // TODO: If we merge bounding boxes together, I think we can just adjust the whole model bounding box
    for (auto const& mesh : m_meshes)
        mesh->adjust_bounding_box(entity->transform->get_model_matrix());

    if (!m_meshes.empty())
        bounds = m_meshes[0]->bounds;
}

BoundingBox Model::get_adjusted_bounding_box(glm::mat4 const& model_matrix) const
{
    if (!m_meshes.empty())
        return m_meshes[0]->get_adjusted_bounding_box(model_matrix);

    return {};
}

Model::Model(std::string const& model_path, std::shared_ptr<Material> const& material)
    : Drawable(material), m_model_path(model_path)
{
}

Model::Model(std::shared_ptr<Material> const& material) : Drawable(material)
{
}

std::string Model::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Model::draw() const
{
    for (auto const& mesh : m_meshes)
        mesh->draw();
}

void Model::draw_instanced(i32 const size)
{
    for (auto const& mesh : m_meshes)
        mesh->draw_instanced(size);
}

void Model::prepare()
{
    if (m_material->is_gpu_instanced)
    {
        if (m_material->first_drawable != nullptr)
            return;

        m_material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    load_model(m_model_path);
}

void Model::reset()
{
    m_meshes.clear();
    m_loaded_textures.clear();
}

void Model::reprepare()
{
    Model::reset();
    Model::prepare();
}

void Model::load_model(std::string const& path)
{
    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
    {
        std::cout << "Error. Failed loading a model: " << importer.GetErrorString() << "\n";
        return;
    }

    m_directory = path.substr(0, path.find_last_of('/'));

    proccess_node(scene->mRootNode, scene);
}

void Model::proccess_node(aiNode const* node, aiScene const* scene)
{
    for (u32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.emplace_back(proccess_mesh(mesh, scene));
    }

    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        proccess_node(node->mChildren[i], scene);
    }
}

std::shared_ptr<Mesh> Model::proccess_mesh(aiMesh const* mesh, aiScene const* scene)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;

    for (u32 i = 0; i < mesh->mNumVertices; ++i)
    {
        Vertex vertex = {};

        vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals())
        {
            vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        if (mesh->mTextureCoords[0] != nullptr)
        {
            vertex.texture_coordinates = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else
        {
            vertex.texture_coordinates = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (u32 i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace const face = mesh->mFaces[i];
        for (u32 k = 0; k < face.mNumIndices; k++)
        {
            indices.push_back(face.mIndices[k]);
        }
    }

    aiMaterial const* assimp_material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuse_maps = load_material_textures(assimp_material, aiTextureType_DIFFUSE, TextureType::Diffuse);
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps = load_material_textures(assimp_material, aiTextureType_SPECULAR, TextureType::Specular);
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    return MeshFactory::create(vertices, indices, textures, m_draw_type, m_material);
}

std::vector<Texture> Model::load_material_textures(aiMaterial const* material, aiTextureType const type, TextureType const type_name)
{
    std::vector<Texture> textures;

    u32 const material_count = material->GetTextureCount(type);
    for (u32 i = 0; i < material_count; ++i)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        bool is_already_loaded = false;
        for (const auto& loaded_texture : m_loaded_textures)
        {
            if (std::strcmp(loaded_texture.path.data(), str.C_Str()) == 0)
            {
                textures.push_back(loaded_texture);
                is_already_loaded = true;
                break;
            }
        }

        if (is_already_loaded)
            continue;

        auto file_path = std::string(str.C_Str());
        file_path = m_directory + '/' + file_path;

        TextureSettings settings = {};
        settings.flip_vertically = false;

        Texture texture = TextureLoader::get_instance()->load_texture(file_path, type_name, settings);
        textures.push_back(texture);
        m_loaded_textures.push_back(texture);
    }

    return textures;
}
