#pragma once
#include "GameObject2D.h"

class PositionOnlyGameObject2D : public GameObject2D
{
public:
	const XMMATRIX& GetWorldMatrix();
protected:
	void UpdateMatrix() override;
	XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
};