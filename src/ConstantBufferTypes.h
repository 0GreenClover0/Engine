#pragma once
#include <DirectXMath.h>

struct ConstantBuffer
{

};

struct ConstantBuffer_Skybox : public ConstantBuffer
{
	DirectX::XMMATRIX PVnoTranslation;
};