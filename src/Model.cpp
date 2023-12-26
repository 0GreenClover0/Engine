#include "Model.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>
#include <glad/glad.h>

#include "Entity.h"
#include "Mesh.h"
#include "Texture.h"
#include "Vertex.h"
#include "imgui_impl/imgui_impl_opengl3_loader.h"

std::shared_ptr<Model> Model::create(std::string model_path, std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(model_path, material);
    model->prepare();

    return model;
}

std::shared_ptr<Model> Model::create(std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(material);

    return model;
}

Model::Model(std::string model_path, std::shared_ptr<Material> const& material) : Drawable(material), model_path(std::move(model_path))
{
    draw_type = GL_TRIANGLES;
}

Model::Model(std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Model::calculate_bounding_box()
{
    for (auto& mesh : meshes)
        mesh.calculate_bounding_box();

    // TODO: Merge bounding boxes together
    if (!meshes.empty())
        bounds = meshes[0].bounds;
}

void Model::adjust_bounding_box()
{
    // TODO: If we merge bounding boxes together, I think we can just adjust the whole model bounding box
    for (auto& mesh : meshes)
        mesh.adjust_bounding_box(entity->transform->get_model_matrix());

    if (!meshes.empty())
        bounds = meshes[0].bounds;
}

BoundingBox Model::get_adjusted_bounding_box(glm::mat4 const& model_matrix) const
{
    if (!meshes.empty())
        return meshes[0].get_adjusted_bounding_box(model_matrix);

    return {};
}

std::string Model::get_name() const
{
    std::string const name = typeid(decltype(*this)).name();
    return name.substr(6);
}

void Model::draw() const
{
    for (auto const& mesh : meshes)
        mesh.draw();
}

void Model::draw_instanced(int32_t const size)
{
    for (auto const& mesh : meshes)
        mesh.draw_instanced(size);
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
    meshes.clear();
    loaded_textures.clear();
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

    directory = path.substr(0, path.find_last_of('/'));

    proccess_node(scene->mRootNode, scene);
}

void Model::proccess_node(aiNode const* node, aiScene const* scene)
{
    for (std::uint32_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh const* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.emplace_back(proccess_mesh(mesh, scene));
    }

    for (std::uint32_t i = 0; i < node->mNumChildren; ++i)
    {
        proccess_node(node->mChildren[i], scene);
    }
}

Mesh Model::proccess_mesh(aiMesh const* mesh, aiScene const* scene)
{
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
    std::vector<Texture> textures;

    for (std::uint32_t i = 0; i < mesh->mNumVertices; ++i)
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

    for (std::uint32_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace const face = mesh->mFaces[i];
        for (std::uint32_t k = 0; k < face.mNumIndices; k++)
        {
            indices.push_back(face.mIndices[k]);
        }
    }

    aiMaterial const* assimp_material = scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuse_maps = load_material_textures(assimp_material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps = load_material_textures(assimp_material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    return Mesh::create(vertices, indices, textures, draw_type, material);
}

std::vector<Texture> Model::load_material_textures(aiMaterial const* material, aiTextureType const type, std::string const& type_name)
{
    std::vector<Texture> textures;

    std::uint32_t const material_count = material->GetTextureCount(type);
    for (std::uint32_t i = 0; i < material_count; ++i)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        bool is_already_loaded = false;
        for (const auto& loaded_texture : loaded_textures)
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

        Texture texture;
        texture.id = texture_from_file(str.C_Str(), directory);
        texture.type = type_name;
        texture.path = str.C_Str();
        textures.push_back(texture);
        loaded_textures.push_back(texture);
    }

    return textures;
}

Texture Model::load_texture(std::string const& path, std::string const& type)
{
    Texture texture;
    texture.id = texture_from_file(path.c_str());
    texture.type = type;
    texture.path = path;
    return texture;
}

std::uint32_t Model::texture_from_file(char const* texture_name, std::string const& directory, bool gamma)
{
    auto file_name = std::string(texture_name);
    file_name = directory + '/' + file_name;

    return load_texture(file_name.c_str(), gamma);
}

std::uint32_t Model::texture_from_file(char const* path, bool gamma)
{
    return load_texture(path, gamma);
}

std::uint32_t Model::load_texture(char const* path, bool gamma)
{
    std::uint32_t texture_id;
    glGenTextures(1, &texture_id);

    stbi_set_flip_vertically_on_load(true);

    std::int32_t width, height, number_of_components;
    unsigned char* data = stbi_load(path, &width, &height, &number_of_components, 0);

    if (data == nullptr)
    {
        std::cout << "Texture failed to load at path: " << path << '\n';
        stbi_image_free(data);
        return texture_id;
    }

    GLenum format;
    if (number_of_components == 1)
    {
        format = GL_RED;
    }
    else if (number_of_components == 3)
    {
        format = GL_RGB;
    }
    else if (number_of_components == 4)
    {
        format = GL_RGBA;
    }
    else
    {
        std::cout << "Unknown texture format. Assuming RGBA." << '\n';
        format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    return texture_id;
}
