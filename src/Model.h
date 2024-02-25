#pragma once
#include <string>
#include <vector>

#include "Renderer.h"
#include "assimp/material.h"

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

    static std::uint32_t load_texture(char const* path, bool gamma = false);
    virtual void draw() const override;

    virtual void draw_instanced(int32_t const size) override;

    virtual void prepare();
    virtual void reset();
    virtual void reprepare();

    void calculate_bounding_box() override;
    void adjust_bounding_box() override;
    BoundingBox get_adjusted_bounding_box(glm::mat4 const& model_matrix) const override;

    // TODO: Move this inside the material
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);

protected:
    explicit Model(std::string const& model_path, std::shared_ptr<Material> const& material);
    explicit Model(std::shared_ptr<Material> const& material);

    [[nodiscard]] static Texture load_texture(std::string const& path, std::string const& type);
    static std::uint32_t texture_from_file(char const* path, bool gamma = false);
    static std::uint32_t texture_from_file(char const* texture_name, std::string const& directory, bool gamma = false);

    DrawType draw_type = DrawType::Triangles;
    std::vector<std::shared_ptr<Mesh>> meshes = {};

private:

    void load_model(std::string const& path);
    void proccess_node(aiNode const* node, aiScene const* scene);
    std::shared_ptr<Mesh> proccess_mesh(aiMesh const* mesh, aiScene const* scene);
    std::vector<Texture> load_material_textures(aiMaterial const* material, aiTextureType type, std::string const& type_name);

    std::string directory;
    std::string model_path;
    std::vector<Texture> loaded_textures;

    friend class SceneSerializer;
};
