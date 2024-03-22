#include "VertexBufferDX11.h"

VertexBufferDX11::VertexBufferDX11(ID3D11Device* device, Vertex const* data, u32 const vertices_count) : m_buffer_size(vertices_count)
{
    m_stride = sizeof(Vertex);

    D3D11_BUFFER_DESC vertex_buffer_desc = {};
    vertex_buffer_desc.ByteWidth = sizeof(Vertex) * vertices_count;
    vertex_buffer_desc.Usage = D3D11_USAGE_IMMUTABLE;
    vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertex_buffer_data = {};
    vertex_buffer_data.pSysMem = data;

    HRESULT const hr = device->CreateBuffer(&vertex_buffer_desc, &vertex_buffer_data, &m_buffer);

    assert(SUCCEEDED(hr));
}

VertexBufferDX11::~VertexBufferDX11()
{
    if (m_buffer)
        m_buffer->Release();
}

ID3D11Buffer* VertexBufferDX11::get() const
{
    return m_buffer;
}

ID3D11Buffer* const* VertexBufferDX11::get_address_of() const
{
    return &m_buffer;
}

u32 VertexBufferDX11::buffer_size() const
{
    return m_buffer_size;
}

u32 VertexBufferDX11::stride() const
{
    return m_stride;
}

u32 const* VertexBufferDX11::stride_ptr() const
{
    return &m_stride;
}
