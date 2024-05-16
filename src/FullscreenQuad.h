#pragma once

#include "Drawable.h"
#include "IndexBufferDX11.h"
#include "VertexBufferDX11.h"

class FullscreenQuad
{
public:
    static std::shared_ptr<FullscreenQuad> create();

    explicit FullscreenQuad(AK::Badge<FullscreenQuad>);

    static std::shared_ptr<FullscreenQuad> get_instance();

    void draw() const;

private:
    static void set_instance(std::shared_ptr<FullscreenQuad> const& fullscreen_quad);

    inline static std::shared_ptr<FullscreenQuad> m_instance;

    std::shared_ptr<IndexBufferDX11> m_index_buffer;
    std::shared_ptr<VertexBufferDX11> m_vertex_buffer;
};
