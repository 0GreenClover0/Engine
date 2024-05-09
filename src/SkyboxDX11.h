#pragma once

#include <memory>

#include "Skybox.h"
#include "SkyboxFactory.h"
#include "VertexBufferDX11.h"
#include "IndexBufferDX11.h"
#include "TextureLoader.h"

class SkyboxDX11 final : public Skybox
{
public:
    SkyboxDX11(AK::Badge<SkyboxFactory>, std::shared_ptr<Material> const& material, std::string const& path);

    virtual void bind() override;
    virtual void unbind() override;
    virtual void draw() const override;
    virtual void update() override;
    virtual void awake() override;

private:
    virtual void bind_texture() const override;
    virtual void unbind_texture() const;
    virtual void create_cube() override;

    std::shared_ptr<VertexBufferDX11> m_vertex_buffer = nullptr;
    std::shared_ptr<IndexBufferDX11> m_index_buffer = nullptr;

    std::shared_ptr<Texture> m_texture = nullptr;
};
