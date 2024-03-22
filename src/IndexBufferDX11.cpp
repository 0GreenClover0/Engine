#include "IndexBufferDX11.h"

#include <cassert>

IndexBufferDX11::IndexBufferDX11(ID3D11Device* device, u32 const* data, u32 const indices_count) : m_buffer_size(indices_count)
{
    // Load Index Data
    D3D11_BUFFER_DESC index_buffer_desc = {};
    index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    index_buffer_desc.ByteWidth = sizeof(u32) * indices_count;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA index_buffer_data = {};
    index_buffer_data.pSysMem = data;

    HRESULT const hr = device->CreateBuffer(&index_buffer_desc, &index_buffer_data, &m_buffer);

    assert(SUCCEEDED(hr));
}

ID3D11Buffer* IndexBufferDX11::get() const
{
    return m_buffer;
}

ID3D11Buffer* const* IndexBufferDX11::get_address_of() const
{
    return &m_buffer;
}

u32 IndexBufferDX11::buffer_size() const
{
    return m_buffer_size;
}
