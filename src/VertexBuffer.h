#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <memory>

#include "AK/Types.h"

template<class T>
class VertexBuffer
{
private:
	ID3D11Buffer* buffer;
	std::shared_ptr<u32> stride;
	u32 bufferSize = 0;

public:
	VertexBuffer() {}

	VertexBuffer(const VertexBuffer<T>& rhs)
	{
		this->buffer = rhs.buffer;
		this->bufferSize = rhs.bufferSize;
		this->stride = rhs.stride;
	}

	VertexBuffer<T>& operator=(const VertexBuffer<T>& a)
	{
		this->buffer = a.buffer;
		this->bufferSize = a.bufferSize;
		this->stride = a.stride;
		return *this;
	}

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

	u32 Stride() const
	{
		return *this->stride.get();
	}

	const u32* StridePtr() const
	{
		return this->stride.get();
	}

	HRESULT Initialize(ID3D11Device* device, const T* data, u32 numVertices)
	{
		if (buffer != nullptr)
			ZeroMemory(&buffer, sizeof(buffer));

		this->bufferSize = numVertices;
		if (this->stride.get() == nullptr)
			this->stride = std::make_shared<u32>(sizeof(T));

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(T) * numVertices;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexBufferData;
		ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
		vertexBufferData.pSysMem = data;

		const HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &buffer);
		return hr;
	}
};
