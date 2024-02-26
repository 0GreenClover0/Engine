#pragma once

#include <vector>

#include "Drawable.h"
#include "Vertex.h"

// TODO: Make skybox more performant
class Skybox : public Drawable
{
public:
    Skybox(std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);

    virtual std::string get_name() const override;

    virtual void draw() const override = 0;
    void virtual bind() = 0;

    static void set_instance(std::shared_ptr<Skybox> const& skybox)
    {
        m_instance = skybox;
    }

    static std::shared_ptr<Skybox> get_instance()
    {
        return m_instance;
    }

    Skybox(Skybox const&) = delete;
    void operator=(Skybox const&) = delete;

protected:
    u32 m_texture_id = 0;

private:
    void virtual bind_texture() const = 0;
    void virtual create_cube() = 0;
    void load_textures();

    inline static std::shared_ptr<Skybox> m_instance;

    std::vector<std::string> m_face_paths;
};
