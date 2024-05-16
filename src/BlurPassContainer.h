#pragma once

#include "RenderPassResourceContainer.h"

#include <memory>

#include <d3d11.h>

#include "AK/Badge.h"

class BlurPassContainer final : public RenderPassResourceContainer
{
public:
    static std::shared_ptr<BlurPassContainer> create();

    explicit BlurPassContainer(AK::Badge<BlurPassContainer>);

    virtual void clear_render_targets() const override;
    virtual void bind_render_targets() const override;
    virtual void bind_shader_resources() const override;
    virtual void update() override;

private:
    ID3D11RenderTargetView* m_blur_render_target = nullptr;
    ID3D11Texture2D* m_blurred_texture = nullptr;
    ID3D11ShaderResourceView* m_blur_srv = nullptr;
};
