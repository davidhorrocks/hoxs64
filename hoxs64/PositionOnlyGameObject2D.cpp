#include "PositionOnlyGameObject2D.h"

void PositionOnlyGameObject2D::UpdateMatrix()
{
	this->worldMatrix = XMMatrixScaling(scale.x, scale.y, 1.0f) * XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z) * XMMatrixTranslation(pos.x + scale.x / 2.0f, pos.y + scale.y / 2.0f, pos.z);
}

const XMMATRIX& PositionOnlyGameObject2D::GetWorldMatrix()
{
	return this->worldMatrix;
}