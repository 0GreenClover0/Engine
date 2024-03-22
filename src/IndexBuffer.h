#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "AK/Types.h"

class IndexBuffer
{
private:
	IndexBuffer(const IndexBuffer& rhs);
	ID3D11Buffer* buffer;
	u32 bufferSize = 0;

public:
	IndexBuffer() {}

	ID3D11Buffer* Get() const
	{
		return buffer;
	}

	ID3D11Buffer* const* GetAddressOf() const
	{
		return &buffer;
	}

	u32 BufferSize() const
	{
		return this->bufferSize;
	}

	HRESULT Initialize(ID3D11Device* device, const DWORD* data, UINT numIndices)
	{
		if (buffer != nullptr)
			ZeroMemory(&buffer, sizeof(buffer));

		this->bufferSize = numIndices;
		//Load Index Data
		D3D11_BUFFER_DESC indexBufferDesc;
		ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(DWORD) * numIndices;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA indexBufferData;
		indexBufferData.pSysMem = data;
		const HRESULT hr = device->CreateBuffer(&indexBufferDesc, &indexBufferData, &buffer);
		return hr;
	}
};
