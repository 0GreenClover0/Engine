#include "IndexBufferDX11.h"

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

HRESULT IndexBufferDX11::initialize(ID3D11Device* device, u32 const* data, u32 const num_indices)
{
    m_buffer_size = num_indices;

    // Load Index Data
    D3D11_BUFFER_DESC index_buffer_desc;
    ZeroMemory(&index_buffer_desc, sizeof(index_buffer_desc));
    index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    index_buffer_desc.ByteWidth = sizeof(u32) * num_indices;
    index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_buffer_desc.CPUAccessFlags = 0;
    index_buffer_desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA index_buffer_data;
    index_buffer_data.pSysMem = data;

    HRESULT const hr = device->CreateBuffer(&index_buffer_desc, &index_buffer_data, &m_buffer);
    return hr;
}
