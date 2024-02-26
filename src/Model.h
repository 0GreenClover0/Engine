#pragma once
#include <string>
#include <vector>

#include <assimp/material.h>

struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;

#include "Texture.h"
#include "Mesh.h"
#include "AK/Badge.h"

class Model : public Drawable
{
public:
    static std::shared_ptr<Model> create(std::string const& model_path, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::shared_ptr<Material> const& material);

    Model() = delete;
    explicit Model(AK::Badge<Model>, std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(AK::Badge<Model>, std::shared_ptr<Material> const& material);

    std::string get_name() const override;

    void draw() const override;

    void draw_instanced(int32_t const size) override;

    virtual void prepare();
    virtual void reset();
    virtual void reprepare();

    void calculate_bounding_box() override;
    void adjust_bounding_box() override;
    BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const override;

protected:
    explicit Model(std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(std::shared_ptr<Material> const& material);

    DrawType m_draw_type = DrawType::Triangles;
    std::vector<std::shared_ptr<Mesh>> m_meshes = {};

private:

    void load_model(std::string const& path);
    void proccess_node(aiNode const* node, aiScene const* scene);
    std::shared_ptr<Mesh> proccess_mesh(aiMesh const* mesh, aiScene const* scene);
    std::vector<Texture> load_material_textures(aiMaterial const* material, aiTextureType type, TextureType const type_name);

    std::string m_directory;
    std::string m_model_path;
    std::vector<Texture> m_loaded_textures;

    friend class SceneSerializer;
};
