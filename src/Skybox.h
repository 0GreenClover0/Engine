#pragma once

#include <vector>

#include "Drawable.h"
#include "Texture.h"
#include "Vertex.h"

// TODO: Make skybox more performant
NON_SERIALIZED
class Skybox : public Drawable
{
public:
    Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);
    Skybox(std::shared_ptr<Material> const& material, std::string const& path);

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void draw() const override = 0;
    void virtual bind() = 0;
    void virtual unbind() = 0;

    static void set_instance(std::shared_ptr<Skybox> const& skybox);
    static std::shared_ptr<Skybox> get_instance();

    Skybox(Skybox const&) = delete;
    void operator=(Skybox const&) = delete;

protected:
    std::shared_ptr<Texture> m_texture = nullptr;

private:
    void virtual bind_texture() const = 0;
    void virtual create_cube() = 0;
    void load_textures();

    inline static std::shared_ptr<Skybox> m_instance;

    std::string m_path = {};
    std::vector<std::string> m_face_paths = {};
};
