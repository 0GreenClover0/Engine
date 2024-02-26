#pragma once

#include <vector>

#include "Drawable.h"
#include "Vertex.h"

// TODO: Make skybox more performant
class Skybox : public Drawable
{
public:
    Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);

    std::string get_name() const override;

    void draw() const override = 0;
    void virtual bind() = 0;

    static void set_instance(std::shared_ptr<Skybox> const& skybox)
    {
        instance = skybox;
    }

    static std::shared_ptr<Skybox> get_instance()
    {
        return instance;
    }

    Skybox(Skybox const&) = delete;
    void operator=(Skybox const&) = delete;

protected:
    uint32_t texture_id = 0;

private:
    void virtual bind_texture() const = 0;
    void virtual create_cube() = 0;
    void load_textures();

    inline static std::shared_ptr<Skybox> instance;

    std::vector<std::string> face_paths;
};
