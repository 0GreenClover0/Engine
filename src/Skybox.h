#pragma once

#include <vector>

#include "Drawable.h"
#include "Vertex.h"

// TODO: Make skybox more performant
class Skybox final : public Drawable
{
public:
    Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> face_paths);

    std::string get_name() const override;

    virtual void draw() const override;
private:
    void create_cube();
    void load_textures();
    void setup_mesh();

    uint32_t texture_id = 0;
    std::vector<std::string> face_paths;
    std::uint32_t VAO = 0, VBO = 0, EBO = 0;
};
