#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>

#include "assimp/material.h"

struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;

#include "Texture.h"
#include "Mesh.h"
#include "Shader.h"
#include "Vertex.h"

class Model : public Drawable
{
public:
    Model() = delete;
    explicit Model(std::string model_path, std::shared_ptr<Material> const& material);
    explicit Model(std::shared_ptr<Material> const& material);

    virtual void draw() const override;

    virtual void prepare();
    virtual void reset();
    virtual void reprepare();

    // TODO: Move this inside the material
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

protected:
    static std::uint32_t texture_from_file(char const* path, bool gamma = false);
    static std::uint32_t texture_from_file(char const* texture_name, std::string const& directory, bool gamma = false);

    GLenum draw_type = GL_TRIANGLES;
    std::vector<Mesh> meshes = {};

private:

    static std::uint32_t load_texture(char const* path, bool gamma = false);
    void load_model(std::string const& path);
    void proccess_node(aiNode const* node, aiScene const* scene);
    Mesh proccess_mesh(aiMesh const* mesh, aiScene const* scene);
    std::vector<Texture> load_material_textures(aiMaterial const* material, aiTextureType type, std::string const& type_name);

    std::string directory;
    std::string model_path;
    std::vector<Texture> loaded_textures;
};
