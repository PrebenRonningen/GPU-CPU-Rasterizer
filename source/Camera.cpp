#include "pch.h"
#include "Camera.h"

const float Camera::m_MovementSpeed = 10.f;
const float Camera::m_RotationSpeed = 1.f;
const float Camera::m_SprintSpeed = 75.f;

size_t Camera::m_AmountOfCameras = 0;

Camera::Camera(const Elite::FPoint3& position, const Elite::FVector3& forwardVector, const float& aspectRatio)
	: m_Position{ position }
	, m_WorldUp{ Elite::FVector3(0,1,0) }
	, m_W{ forwardVector }
	, m_FOV{ 45.f }
	, m_S{ 1 }
	, m_AspectRatio{ aspectRatio }
	, m_ID{ ++m_AmountOfCameras }
	, m_NearPlane{ 0.1f }
	, m_FarPlane{ 1000.0f }
	, m_RProjectionMatrix{ Elite::FMatrix4::Identity() }
	, m_LProjectionMatrix{ Elite::FMatrix4::Identity() }
{
	m_V = m_WorldUp;

	SetFOV(m_FOV);
	SetONBMatrix(m_WorldUp);
	FlipCamera();
	SetRightHandedProjectionMatrix();
	SetLeftHandedProjectionMatrix();
}

Camera::~Camera()
{
}

void Camera::SetONBMatrix(const Elite::FVector3& worldUp)
{
	m_U = Elite::Cross(worldUp, m_W);
	m_V = Elite::Cross(m_W, m_U);
	m_W = Elite::Cross(m_U, m_V);

	Elite::Normalize(m_U);
	Elite::Normalize(m_V);
	Elite::Normalize(m_W);

	m_ONB = Elite::FMatrix4(m_U, m_V, m_W, Elite::FVector4(m_Position.x, m_Position.y, m_Position.z, 1));
	m_InverseONB = Elite::Inverse(m_ONB);
	m_RotationMatrix = Elite::FMatrix3(m_U, m_V, m_W);
}

void Camera::SetRightHandedProjectionMatrix()
{
	m_RProjectionMatrix[0][0] = 1.0f / ( m_AspectRatio * m_FOV );
	m_RProjectionMatrix[1][1] = 1.0f / m_FOV ;
	m_RProjectionMatrix[2][2] = m_FarPlane / ( m_NearPlane - m_FarPlane );
	m_RProjectionMatrix[3][2] = ( m_FarPlane * m_NearPlane ) / ( m_NearPlane - m_FarPlane );
	m_RProjectionMatrix[2][3] = -1.0f;
	m_RProjectionMatrix[3][3] = 0.0f;
}

void Camera::SetLeftHandedProjectionMatrix()
{
	m_LProjectionMatrix[0][0] = 1.0f / ( m_AspectRatio * m_FOV );
	m_LProjectionMatrix[1][1] = 1.0f / m_FOV;
	m_LProjectionMatrix[2][2] = m_FarPlane / ( m_FarPlane - m_NearPlane );
	m_LProjectionMatrix[3][2] = -( m_FarPlane * m_NearPlane ) / ( m_FarPlane - m_NearPlane );
	m_LProjectionMatrix[2][3] = 1.0f;
	m_LProjectionMatrix[3][3] = 0.0f;
}

const Elite::FPoint3& Camera::GetPosition() const
{
	return m_Position;
}

const Elite::FMatrix4& Camera::GetInverseViewMatrix() const
{
	return m_InverseONB;
}

const Elite::FMatrix4& Camera::GetProjectionMatrix() const
{
	return ( m_RightHanded ) ? m_RProjectionMatrix : m_LProjectionMatrix;
}

void Camera::FlipCamera()
{
		m_RightHanded = !m_RightHanded;
		m_Position.z *= -1;
		m_W.x *= -1;
		m_W.y *= -1;
		SetONBMatrix(m_WorldUp);
}

#pragma region Movement

#pragma region Keyboard
void Camera::MoveLocalX(const float deltaTime, const bool positiveDirection)
{
	m_Position += ( m_U * deltaTime * m_ActiveMovementSpeed * ( positiveDirection ? 1.f : -1.f ) );
	SetONBMatrix(m_V);
}
void Camera::MoveLocalY(const float deltaTime, const bool positiveDirection)
{
	m_Position += ( m_V * deltaTime * m_ActiveMovementSpeed * ( positiveDirection ? 1.f : -1.f ) );
	SetONBMatrix(m_V);
}

void Camera::MoveLocalZ(const float deltaTime, const bool positiveDirection)
{
	m_Position += ( m_W * deltaTime * m_ActiveMovementSpeed * ( ( m_RightHanded ) ? ( positiveDirection ? 1.f : -1.f ) : ( positiveDirection ? -1.f : 1.f ) ) );
	SetONBMatrix(m_V);
}

void Camera::Pan(const float deltaTime, const bool positiveDirection)
{
	m_W = MakeRotation(m_RotationSpeed * deltaTime * ( ( m_RightHanded ) ? ( positiveDirection ? 1.f : -1.f ) : ( positiveDirection ? -1.f : 1.f ) ), m_WorldUp) * m_W;
	SetONBMatrix(m_WorldUp);
}

void Camera::Tilt(const float deltaTime, const bool positiveDirection)
{
	if ( m_W.y >= -( 1 - FLT_EPSILON ) && m_W.y <= ( 1 - FLT_EPSILON ) )
	{
		m_W = MakeRotation(m_RotationSpeed * deltaTime * ( ( m_RightHanded ) ? ( positiveDirection ? 1.f : -1.f ) : ( positiveDirection ? -1.f : 1.f ) ), m_U) * m_W;
		SetONBMatrix(m_V);
	}
}
void Camera::Sprint(bool isSprinting)
{
	if ( isSprinting )
	{
		m_ActiveMovementSpeed = m_SprintSpeed;
	} else
	{
		m_ActiveMovementSpeed = m_MovementSpeed;
	}
}

#pragma endregion

#pragma region MOUSE
void Camera::RotateAndMoveLRMouse(const float deltaTime, const int x, const int y)
{
	m_Position += m_W * deltaTime * m_ActiveMovementSpeed * float(-y) * (( m_RightHanded ) ? 1.f : -1.f );
	m_W = MakeRotation(deltaTime * ( x < 0 ? 1.f : ( x > 0 ) ? -1.f : 0 ) * ( ( m_RightHanded ) ? 1.f : -10.f ), m_WorldUp) * m_W;
	SetONBMatrix(m_WorldUp);
}

void Camera::RotateMouse(const float deltaTime, const int x, const int y)
{
	m_W = MakeRotation(deltaTime * ( x < 0 ? 1.f : ( x > 0 ) ? -1.f : 0 ) * ( ( m_RightHanded ) ? 1.f : -10.f ), m_WorldUp) * m_W;
	SetONBMatrix(m_WorldUp);
	if ( m_W.y >= -( 1 - FLT_EPSILON ) && m_W.y <= ( 1 - FLT_EPSILON ) )
	{
		m_W = MakeRotation(deltaTime * ( y < 0 ? 1.f : ( y > 0 ) ? -1.f : 0 ) * ( ( m_RightHanded ) ? 1.f : -10.f ), m_U) * m_W;
	}
	SetONBMatrix(m_V);
}

void Camera::MoveUpDownMouse(const float deltaTime, const int y)
{
	if ( y == 0 ) return;

	m_Position.y += deltaTime * float(y) * (( m_RightHanded ) ? 1.f : 10.f) * m_ActiveMovementSpeed;
	SetONBMatrix(m_WorldUp);
}

#pragma endregion

#pragma endregion

#pragma region CameraInfo

void Camera::SetFOV(const float degrees)
{
	m_FOV = tanf(Elite::ToRadians(degrees) / 2.f);
	SetRightHandedProjectionMatrix();
	SetLeftHandedProjectionMatrix();
}

float Camera::GetFOV() const
{
	return float(E_TO_DEGREES * atanf(m_FOV) * 2.f);
}
void Camera::ResetFOV()
{
	SetFOV(45.f);
}
const size_t Camera::GetID() const
{
	return m_ID;
}

const char* Camera::GetType() const
{
	return "Camera";
}

#pragma endregion