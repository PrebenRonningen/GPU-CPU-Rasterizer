#include "pch.h"
#include "Object.h"

size_t Object::m_AmountOfObjects = 0;

Object::Object(const Elite::FPoint3& position)
	: m_ID{ ++m_AmountOfObjects }
	, m_Position{ position }
	, m_W{ Elite::FVector3(0.0f, 0.0f, 1.0f) }
{
	SetWorldMatrix(Elite::FVector3(0.0f, 1.0f, 0.0f));
}

void Object::SetWorldMatrix(const Elite::FVector3& worldUp)
{
	m_U = Elite::Cross(worldUp, m_W);
	m_V = Elite::Cross(m_W, m_U);
	m_W = Elite::Cross(m_U, m_V);

	Elite::Normalize(m_U);
	Elite::Normalize(m_V);
	Elite::Normalize(m_W);

	m_TransformationMatrix = Elite::FMatrix4(m_U, m_V, m_W, Elite::FVector4(m_Position.x, m_Position.y, m_Position.z, 1));
	m_RotationMatrix = Elite::FMatrix4(m_U, m_V, m_W, Elite::FVector4(0, 0, 0, 1));
	m_TransposedTransformationMatrix = Elite::Inverse(m_TransformationMatrix);
}

const Elite::FMatrix4& Object::GetTransformationMatrix() const
{
	return m_TransformationMatrix;
}

const Elite::FMatrix4& Object::GetTransposedTransformationMatrix()
{
	Elite::FMatrix4 rotMat = Elite::Transpose(m_RotationMatrix);



	Elite::FMatrix4 mat = Elite::FMatrix4(Elite::FMatrix4::Identity()[0], Elite::FMatrix4::Identity()[1], Elite::FMatrix4::Identity()[2], Elite::FVector4(m_Position.x, m_Position.y, -m_Position.z, 1));
	m_TransposedTransformationMatrix = mat * rotMat;

	return m_TransposedTransformationMatrix;
}

void Object::SetForwardVectpr(const Elite::FVector3& fVector)
{
	m_W = fVector;
	SetWorldMatrix(m_V);
}