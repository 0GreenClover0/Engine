#pragma once
#include <string>
#include <vector>

#include <assimp/material.h>

#include "Texture.h"
#include "Mesh.h"
#include "AK/Badge.h"

struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiNode;

class Model : public Drawable
{
public:
    static std::shared_ptr<Model> create();
    static std::shared_ptr<Model> create(std::string const& model_path, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::shared_ptr<Material> const& material);

    explicit Model(AK::Badge<Model>, std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(AK::Badge<Model>, std::shared_ptr<Material> const& material);

    virtual void draw_editor() override;

    virtual void draw() const override;

    virtual void draw_instanced(i32 const size) override;

    virtual void prepare();
    virtual void reset();
    virtual void reprepare() override;

    virtual void calculate_bounding_box() override;
    virtual void adjust_bounding_box() override;
    virtual BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const override;

    std::string model_path = "";

protected:
    explicit Model(std::shared_ptr<Material> const& material);

    DrawType m_draw_type = DrawType::Triangles;
    std::vector<std::shared_ptr<Mesh>> m_meshes = {};

private:
    void load_model(std::string const& path);
    void proccess_node(aiNode const* node, aiScene const* scene);
    std::shared_ptr<Mesh> proccess_mesh(aiMesh const* mesh, aiScene const* scene);
    std::vector<std::shared_ptr<Texture>> load_material_textures(aiMaterial const* material, aiTextureType type, TextureType const type_name);

    std::string m_directory;
    std::vector<std::shared_ptr<Texture>> m_loaded_textures;
};
