#include "Model.h"

#include "AK/Types.h"
#include "Entity.h"
#include "Globals.h"
#include "Mesh.h"
#include "MeshFactory.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Vertex.h"

#include <filesystem>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#if EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

std::shared_ptr<Model> Model::create()
{
    auto model = std::make_shared<Model>(AK::Badge<Model> {}, default_material);

    return model;
}

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
    : Drawable(material), model_path(model_path)
{
}

Model::Model(AK::Badge<Model>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

#if EDITOR
void Model::draw_editor()
{
    Drawable::draw_editor();

    ImGui::InputText("Model Path", &model_path);

    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        reprepare();
    }

    // Choose rasterizer draw mode for individual model
    std::array const draw_type_items = {"Default", "Wireframe", "Solid"};

    i32 current_item_index = static_cast<i32>(m_rasterizer_draw_type);
    if (ImGui::Combo("Rasterizer Draw Type", &current_item_index, draw_type_items.data(), draw_type_items.size()))
    {
        m_rasterizer_draw_type = static_cast<RasterizerDrawType>(current_item_index);
    }
}
#endif

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

Model::Model(std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Model::draw() const
{
    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    for (auto const& mesh : m_meshes)
        mesh->draw();

    Renderer::get_instance()->restore_default_rasterizer_draw_type();
}

void Model::draw_instanced(i32 const size)
{
    for (auto const& mesh : m_meshes)
        mesh->draw_instanced(size);
}

void Model::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    load_model(model_path);
}

void Model::reset()
{
    m_meshes.clear();
    m_loaded_textures.clear();
}

void Model::reprepare()
{
    reset();
    prepare();
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

    std::filesystem::path const filesystem_path = path;
    m_directory = filesystem_path.parent_path().string();

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
    std::vector<std::shared_ptr<Texture>> textures;

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

    std::vector<std::shared_ptr<Texture>> diffuse_maps =
        load_material_textures(assimp_material, aiTextureType_DIFFUSE, TextureType::Diffuse);
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<std::shared_ptr<Texture>> specular_maps =
        load_material_textures(assimp_material, aiTextureType_SPECULAR, TextureType::Specular);
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    return ResourceManager::get_instance().load_mesh(m_meshes.size(), model_path, vertices, indices, textures, m_draw_type, material);
}

std::vector<std::shared_ptr<Texture>> Model::load_material_textures(aiMaterial const* material, aiTextureType const type,
                                                                    TextureType const type_name)
{
    std::vector<std::shared_ptr<Texture>> textures;

    u32 const material_count = material->GetTextureCount(type);
    for (u32 i = 0; i < material_count; ++i)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        bool is_already_loaded = false;
        for (auto const& loaded_texture : m_loaded_textures)
        {
            if (std::strcmp(loaded_texture->path.data(), str.C_Str()) == 0)
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
        settings.filtering_min = TextureFiltering::Nearest;
        settings.filtering_max = TextureFiltering::Nearest;
        settings.filtering_mipmap = TextureFiltering::Nearest;

        std::shared_ptr<Texture> texture = ResourceManager::get_instance().load_texture(file_path, type_name, settings);
        textures.push_back(texture);
        m_loaded_textures.push_back(texture);
    }

    return textures;
}
