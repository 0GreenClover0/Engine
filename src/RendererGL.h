#pragma once

#include "Renderer.h"
#include "AK/Badge.h"

class RendererGL final : public Renderer
{
public:
    static std::shared_ptr<RendererGL> create();
    explicit RendererGL(AK::Badge<RendererGL>);

    ~RendererGL() override = default;

    void begin_frame() const override;

private:
    void initialize_global_renderer_settings() override;
    void initialize_buffers(size_t const max_size) override;
    void perform_frustum_culling(std::shared_ptr<Material> const& material) const override;

    GLuint m_gpu_instancing_ssbo = {};
    GLuint m_bounding_boxes_ssbo = {};
    GLuint m_visible_instances_ssbo = {};
};
