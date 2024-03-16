#pragma once

#include <glad/glad.h>

#include "Renderer.h"
#include "AK/Badge.h"

class RendererGL final : public Renderer
{
public:
    static std::shared_ptr<RendererGL> create();
    explicit RendererGL(AK::Badge<RendererGL>);

    ~RendererGL() override = default;

    virtual void begin_frame() const override;

private:
    virtual void initialize_global_renderer_settings() override;
    virtual void initialize_buffers(size_t const max_size) override;
    virtual void perform_frustum_culling(std::shared_ptr<Material> const& material) const override;

    std::shared_ptr<Shader> m_frustum_culling_shader = {};

    GLuint m_gpu_instancing_ssbo = {};
    GLuint m_bounding_boxes_ssbo = {};
    GLuint m_visible_instances_ssbo = {};
};
