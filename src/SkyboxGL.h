#pragma once

#include "Skybox.h"
#include "AK/Types.h"

class SkyboxFactory;

class SkyboxGL final : public Skybox
{
public:
    SkyboxGL(AK::Badge<SkyboxFactory>, std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);

    virtual void bind() override;
    virtual void draw() const override;

private:
    virtual void bind_texture() const override;
    virtual void create_cube() override;

    u32 m_VAO = 0, m_VBO = 0, m_EBO = 0;
};
