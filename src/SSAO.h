#pragma once

#include <array>
#include <memory>

#include <d3d11.h>

#include <glm/glm.hpp>

#include "AK/Badge.h"
#include "RenderPassResourceContainer.h"

class SSAO final : public RenderPassResourceContainer
{
public:
    static std::shared_ptr<SSAO> create();

    explicit SSAO(AK::Badge<SSAO>);

    virtual void clear_render_targets() const override;
    virtual void bind_render_targets() const override;
    virtual void bind_shader_resources() const override;
    virtual void update() override;

    inline static constexpr i32 kernel_size = 64;
    inline static constexpr i32 noise_size = 16;

    [[nodiscard]] std::array<glm::vec4, kernel_size> get_ssao_kernel() const;

private:
    ID3D11RenderTargetView* m_ssao_render_target = {};

    ID3D11Texture2D* m_ssao_kernel_rotations_buffer = nullptr;
    ID3D11Texture2D* m_ssao_texture = nullptr;
    std::array<glm::vec4, kernel_size> m_ssao_kernel = {};

    ID3D11ShaderResourceView* m_ssao_srv = nullptr;
    ID3D11ShaderResourceView* m_ssao_kernel_rotations_srv = nullptr;
};
