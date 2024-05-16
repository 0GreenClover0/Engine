#pragma once

#include "Shader.h"

class RenderPassResourceContainer
{
public:
    virtual ~RenderPassResourceContainer();

    virtual void clear_render_targets() const = 0;
    virtual void bind_render_targets() const = 0;
    virtual void bind_shader_resources() const = 0;
    virtual void update() = 0;
    virtual void use_shader() const;

protected:
    std::shared_ptr<Shader> m_pass_shader = nullptr;
};
