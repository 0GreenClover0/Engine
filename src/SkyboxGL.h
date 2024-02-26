#pragma once

#include "Skybox.h"

class SkyboxFactory;

class SkyboxGL final : public Skybox
{
public:
    SkyboxGL(AK::Badge<SkyboxFactory>, std::shared_ptr<Material> const& material, std::vector<std::string> const& face_paths);

    void bind() override;
    void draw() const override;

private:
    void bind_texture() const override;
    void create_cube() override;

    std::uint32_t VAO = 0, VBO = 0, EBO = 0;
};
