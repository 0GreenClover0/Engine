#pragma once

#include <d3d11.h>

#include "AK/Types.h"

class IndexBufferDX11
{
public:
    IndexBufferDX11() = default;

    [[nodiscard]] ID3D11Buffer* get() const;

    [[nodiscard]] ID3D11Buffer* const* get_address_of() const;

    [[nodiscard]] u32 buffer_size() const;

    HRESULT initialize(ID3D11Device* device, u32 const* data, u32 const num_indices);

private:
    ID3D11Buffer* m_buffer = nullptr;
    u32 m_buffer_size = 0;
};
