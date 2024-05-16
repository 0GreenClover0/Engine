#pragma once

#include <array>
#include <memory>

#include <d3d11.h>

#include "AK/Badge.h"
#include "RenderPassResourceContainer.h"

class GBuffer final : public RenderPassResourceContainer
{
public:
    static std::shared_ptr<GBuffer> create();

    explicit GBuffer(AK::Badge<GBuffer>);

    virtual void clear_render_targets() const override;
    virtual void bind_render_targets() const override;
    virtual void bind_shader_resources() const override;
    virtual void update() override;

private:
    std::array<ID3D11RenderTargetView*, 3> m_gbuffer_rendertargets = {};

    ID3D11Texture2D* m_position_texture = nullptr;
    ID3D11Texture2D* m_normal_texture = nullptr;
    ID3D11Texture2D* m_diffuse_texture = nullptr;

    ID3D11ShaderResourceView* m_position_texture_view = nullptr;
    ID3D11ShaderResourceView* m_normal_texture_view = nullptr;
    ID3D11ShaderResourceView* m_diffuse_texture_view = nullptr;
};
