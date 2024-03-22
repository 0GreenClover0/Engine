#pragma once

#include <d3d11.h>

#include "AK/Types.h"

template<class T>
class VertexBufferDX11
{
public:
    VertexBufferDX11() = default;

    VertexBufferDX11(const VertexBufferDX11<T>& rhs)
    {
        m_buffer = rhs.m_buffer;
        m_buffer_size = rhs.m_buffer_size;
        m_stride = rhs.m_stride;
    }

    VertexBufferDX11<T>& operator=(const VertexBufferDX11<T>& a)
    {
        m_buffer = a.m_buffer;
        m_buffer_size = a.m_buffer_size;
        m_stride = a.m_stride;
        return *this;
    }

    [[nodiscard]] ID3D11Buffer* get() const
    {
        return m_buffer;
    }

    [[nodiscard]] ID3D11Buffer* const* get_address_of() const
    {
        return &m_buffer;
    }

    [[nodiscard]] u32 buffer_size() const
    {
        return m_buffer_size;
    }

    [[nodiscard]] u32 stride() const
    {
        return m_stride;
    }

    [[nodiscard]] u32 const* stride_ptr() const
    {
        return &m_stride;
    }

    HRESULT initialize(ID3D11Device* device, T const* data, u32 const num_vertices)
    {
        m_buffer_size = num_vertices;
        m_stride = sizeof(T);

        D3D11_BUFFER_DESC vertexBufferDesc;
        ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.ByteWidth = sizeof(T) * num_vertices;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA vertex_buffer_data;
        ZeroMemory(&vertex_buffer_data, sizeof(vertex_buffer_data));
        vertex_buffer_data.pSysMem = data;

        HRESULT const hr = device->CreateBuffer(&vertexBufferDesc, &vertex_buffer_data, &m_buffer);
        return hr;
    }

private:
    ID3D11Buffer* m_buffer = nullptr;
    u32 m_stride = 0;
    u32 m_buffer_size = 0;
};
