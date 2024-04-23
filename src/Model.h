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

#include "Rig.h"
#include "AK/AnimXForm.h"

enum class LoadMode
{
    Model,
    Rig,
    Animation
};

class Model : public Drawable
{
public:

    // For convenience while the animation system is being developed
    Rig rig;

    // For debugging so we can test skinning using sin(time)
    float time = 0;

    std::vector<xform> model_pose;
    std::vector<xform> local_pose;

    static std::shared_ptr<Model> create();
    static std::shared_ptr<Model> create(std::string const& model_path, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::string const& model_path, std::string const& anim_path, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> const& material);
    static std::shared_ptr<Model> create(std::shared_ptr<Material> const& material);

    explicit Model(AK::Badge<Model>, std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(AK::Badge<Model>, std::string const& model_path, std::string const& anim_path, std::shared_ptr<Material> const& material);
    explicit Model(AK::Badge<Model>, std::shared_ptr<Material> const& material);

    virtual void draw_editor() override;
    virtual std::string get_name() const override;

    virtual void draw() const override;

    virtual void draw_instanced(i32 const size) override;

    void pre_draw_update(); // Hack for skinning purposes
    virtual void prepare();
    virtual void reset();
    virtual void reprepare() override;

    virtual void calculate_bounding_box() override;
    virtual void adjust_bounding_box() override;
    virtual BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const override;

    bool is_skinned() const;

    std::string model_path = "";
    std::string anim_path = "";

protected:
    explicit Model(std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(std::shared_ptr<Material> const& material);

    DrawType m_draw_type = DrawType::Triangles;
    std::vector<std::shared_ptr<Mesh>> m_meshes = {};

private:

    void load_model(std::string const& path, LoadMode mode);
    void proccess_node(aiNode const* node);
    std::shared_ptr<Mesh> proccess_mesh(aiMesh const* mesh);
    std::vector<Texture> load_material_textures(aiMaterial const* material, aiTextureType type, TextureType const type_name);

    // Skinning
    void extract_bone_data(aiNode* node, LoadMode mode);
    void extract_bone_data_from_mesh(aiMesh* mesh, LoadMode mode);

    const aiScene* m_scene = nullptr;

    std::string m_directory;
    std::vector<Texture> m_loaded_textures;
    bool m_is_skinned = false;
};
