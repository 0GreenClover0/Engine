#pragma once

#include <d3d11.h>

#include "AK/Types.h"
#include "Vertex.h"

class VertexBufferDX11
{
public:
    VertexBufferDX11(ID3D11Device* device, Vertex const* data, u32 const vertices_count);
    VertexBufferDX11(VertexBufferDX11 const& rhs) = delete;
    VertexBufferDX11& operator=(VertexBufferDX11 const& rhs) = delete;

    ~VertexBufferDX11();

    [[nodiscard]] ID3D11Buffer* get() const;

    [[nodiscard]] ID3D11Buffer* const* get_address_of() const;

    [[nodiscard]] u32 buffer_size() const;

    [[nodiscard]] u32 stride() const;

    [[nodiscard]] u32 const* stride_ptr() const;

private:
    ID3D11Buffer* m_buffer = nullptr;
    u32 m_stride = 0;
    u32 m_buffer_size = 0;
};
