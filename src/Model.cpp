#include "Model.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Entity.h"
#include "Globals.h"
#include "Mesh.h"
#include "MeshFactory.h"
#include "Texture.h"
#include "TextureLoader.h"
#include "Vertex.h"
#include "AK/Types.h"

#include <filesystem>
#include <map>

#include "RendererDX11.h"
#include "AK/aiHelpers.h"

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

std::shared_ptr<Model> Model::create(std::string const& model_path, std::string const& anim_path,
    std::shared_ptr<Material> const& material)
{
    auto model = std::make_shared<Model>(AK::Badge<Model> {}, model_path, anim_path, material);
    model->m_is_skinned = true;
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

Model::Model(AK::Badge<Model>, std::string const& model_path, std::string const& anim_path,
    std::shared_ptr<Material> const& material) : Drawable(material), model_path(model_path), anim_path(anim_path)
{
}

Model::Model(AK::Badge<Model>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Model::draw_editor()
{
    Drawable::draw_editor();

    model_path.reserve(255);

    if (ImGui::InputText("Model Path", model_path.data(), 255, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        reprepare();
    }
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

bool Model::is_skinned() const
{
    return m_is_skinned;
}

Model::Model(std::string const& model_path, std::shared_ptr<Material> const& material)
    : Drawable(material), model_path(model_path)
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

void Model::pre_draw_update()
{
    time = glfwGetTime();
    glm::mat4 skinning_matrices[512] = { glm::mat4(1) };

    for (auto const& mesh : m_meshes)
    {
        rig.local_to_model(model_pose, local_pose);

        for (int j = 0; j < rig.num_bones; j++)
        {
            xform inverse_bind_pose_x_form, skinned_x_form;
            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(rig.inverse_bind_pose[j], scale, rotation, translation, skew, perspective);

            inverse_bind_pose_x_form.position = translation;
            inverse_bind_pose_x_form.rotation = rotation;

            skinned_x_form = model_pose[j] * inverse_bind_pose_x_form;

            glm::mat4 skinning_matrix = glm::mat4(1.0f);

            skinning_matrix = glm::translate(skinning_matrix, skinned_x_form.position);
            skinning_matrix = skinning_matrix * glm::toMat4(skinned_x_form.rotation);
            skinning_matrix = glm::scale(skinning_matrix, glm::vec3(1.0f));

            if (j == 4)
            {
                glm::mat4 rot = glm::mat4(1.0f);
                rot = glm::rotate(rot, glm::sin(time), glm::vec3(0, 1, 0));
                skinning_matrices[j] = glm::inverse(rig.inverse_bind_pose[j]) * rot * rig.inverse_bind_pose[j];
            }
            else
                skinning_matrices[j] = glm::inverse(rig.inverse_bind_pose[j]) * rig.inverse_bind_pose[j];
        }

        // Here should be setting matrices/uniform
        for(u16 i = 0; i < 512; ++i)
        {
            rig.skinned_pose[i] = skinning_matrices[i];
        }
        // This is one of the places I have finished...
        // Much of this code is gonna be reworked during loading poses from files and sampling (interpolating) them in time
    }
}

void Model::extract_bone_data(aiNode* node, LoadMode mode)
{
    // Process each mesh node if it contains bones
    if (node->mNumMeshes > 0)
    {
        aiMesh* mesh = m_scene->mMeshes[node->mMeshes[0]]; // Assuming one mesh per node for simplicity
        extract_bone_data_from_mesh(mesh, mode);
    }

    // Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        extract_bone_data(node->mChildren[i], mode);
    }
}

void Model::extract_bone_data_from_mesh(aiMesh* mesh, LoadMode mode)
{
    // Extract bone data from the mesh
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        // Extract bone name, parent index, and bind pose
        const aiBone* bone = mesh->mBones[i];
        hstring parent_bone_name;
        hstring bone_name = hash(bone->mName.C_Str());

        if (bone->mNode == nullptr || bone->mNode->mParent == nullptr)
            parent_bone_name = -1;
        else
            parent_bone_name = hash(bone->mNode->mParent->mName.C_Str());

        // Find parent bone in the mBones array
        int parent_index = -1;

        for (unsigned int j = 0; j < mesh->mNumBones; j++)
        {
            const hstring checked_bone_name = hash(mesh->mBones[j]->mName.C_Str());
            if (i != j && parent_bone_name == checked_bone_name)
            {
                // Found the parent bone
                parent_index = j;
                break;
            }
        }

        const aiMatrix4x4 ai_inverse_bind_pose = bone->mOffsetMatrix;
        glm::mat4 inverse_bind_pose = aiMatrix4x4ToGlm(&ai_inverse_bind_pose);

        rig.bone_names.push_back(bone_name);
        rig.parents.push_back(parent_index);
        rig.inverse_bind_pose.push_back(inverse_bind_pose);
    }
    rig.num_bones += mesh->mNumBones;
}

void Model::prepare()
{
    if (material->is_gpu_instanced)
    {
        if (material->first_drawable != nullptr)
            return;

        material->first_drawable = std::dynamic_pointer_cast<Drawable>(shared_from_this());
    }

    if (m_is_skinned)
    {
        load_model(model_path, LoadMode::Rig);
        load_model(anim_path, LoadMode::Animation);
    }

    load_model(model_path, LoadMode::Model);
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

void Model::load_model(std::string const& path, LoadMode mode)
{
    Assimp::Importer importer;
    m_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_PopulateArmatureData);

    if (m_scene == nullptr || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || m_scene->mRootNode == nullptr)
    {
        std::cout << "Error. Failed loading a model: " << importer.GetErrorString() << "\n";
        return;
    }

    if (mode == LoadMode::Animation)
    {
        if (!m_scene || !m_scene->HasAnimations())
        {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << "::LOADMODE::" << static_cast<int>(mode) << std::endl;
            return;
        }
    }

    std::filesystem::path const filesystem_path = path;
    m_directory = filesystem_path.parent_path().string();

    if (mode == LoadMode::Rig)
    {
        // Extract rig data from the scene
        extract_bone_data(m_scene->mRootNode, mode);

        proccess_node(m_scene->mRootNode);
    }

    if (mode == LoadMode::Animation) // If we are loading animation, not rig
    {
        // Just a pose for now
        // TODO: Cut out to separate function
        // TODO: Attach sampler of course lol

        std::cout << rig.num_bones << std::endl;

        for (int i = 0; i < rig.num_bones; i++)
        {
            xform transform;

            // const aiAnimation* animation = scene->mAnimations[0]; // Get the first animation
            // const aiNodeAnim* channel = animation->mChannels[i]; // Get the first channel
            //
            // // Get the position, rotation, and scaling keyframes from the first keyframe
            // const aiVector3D position = channel->mPositionKeys[0].mValue;
            // const aiQuaternion rotation = channel->mRotationKeys[0].mValue;
            //
            // transform.position = aiPosToGLMVec3(position);
            // transform.rotation = aiQuatToGLMQuat(rotation);

            local_pose.emplace_back(transform);
        }
    }

    proccess_node(m_scene->mRootNode);
}

void Model::proccess_node(aiNode const* node)
{
    for (u32 i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh const* mesh = m_scene->mMeshes[node->mMeshes[i]];
        m_meshes.emplace_back(proccess_mesh(mesh));
    }

    for (u32 i = 0; i < node->mNumChildren; ++i)
    {
        proccess_node(node->mChildren[i]);
    }
}

std::shared_ptr<Mesh> Model::proccess_mesh(aiMesh const* mesh)
{
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
    std::vector<Texture> textures;

    glm::ivec4 skin_indices = { -1, -1, -1, -1 };
    glm::vec4 skin_weights = { 0, 0, 0, 0 };

    std::map<unsigned int, glm::ivec4> vertex_to_bone_indices;
    std::map<unsigned int, glm::vec4> vertex_to_bone_weights;

    Vertex vertex;
    unsigned int vertex_id = 0;
    float weight = 0.0f;

    if (m_is_skinned)
    {
        for (u32 i = 0; i < mesh->mNumBones; i++)
        {
            for (u32 j = 0; j < mesh->mBones[i]->mNumWeights; j++)
            {
                vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
                u32 bone_id = i;
                weight = mesh->mBones[i]->mWeights[j].mWeight;

                // Initialize the ivec4 with -1s and vec4 with 0s - default values

                if (!vertex_to_bone_indices.contains(vertex_id))
                {
                    vertex_to_bone_indices.try_emplace(vertex_id, skin_indices);
                    vertex_to_bone_weights.try_emplace(vertex_id, skin_weights);
                }

                // TODO: Get rid of this way of filling the ivec4.
                // This is just for simplicity because SKINNING IS NOT AN EASY THING
                // And this is the exact moment when we define how the bones affect vertices
                if (vertex_to_bone_indices[vertex_id].x == -1)
                {
                    vertex_to_bone_indices[vertex_id].x = i;
                    vertex_to_bone_weights[vertex_id].x = weight;
                }
                else if (vertex_to_bone_indices[vertex_id].y == -1)
                {
                    vertex_to_bone_indices[vertex_id].y = i;
                    vertex_to_bone_weights[vertex_id].y = weight;
                }
                else if (vertex_to_bone_indices[vertex_id].z == -1)
                {
                    vertex_to_bone_indices[vertex_id].z = i;
                    vertex_to_bone_weights[vertex_id].z = weight;
                }
                else if (vertex_to_bone_indices[vertex_id].w == -1)
                {
                    vertex_to_bone_indices[vertex_id].w = i;
                    vertex_to_bone_weights[vertex_id].w = weight;
                }
            }
        }
    }

    for (u32 i = 0; i < mesh->mNumVertices; ++i)
    {
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

        if (m_is_skinned)
        {
            vertex.skin_indices = vertex_to_bone_indices[i];
            vertex.skin_weights = vertex_to_bone_weights[i];
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

    aiMaterial const* assimp_material = m_scene->mMaterials[mesh->mMaterialIndex];

    std::vector<Texture> diffuse_maps = load_material_textures(assimp_material, aiTextureType_DIFFUSE, TextureType::Diffuse);
    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    std::vector<Texture> specular_maps = load_material_textures(assimp_material, aiTextureType_SPECULAR, TextureType::Specular);
    textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());

    return MeshFactory::create(vertices, indices, textures, m_draw_type, material);
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
