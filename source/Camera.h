#pragma once
#include "EMath.h"
#include "EVector.h"

class Camera
{
public:
	Camera(const Elite::FPoint3& position = Elite::FPoint3(0, 0, 0), const Elite::FVector3& forwardVector = Elite::FVector3(0, 0, -1), const float& aspectRatio = 1.0f);

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;

	~Camera();

	void SetFOV(const float degrees);
	float GetFOV() const;
	void ResetFOV();

	void SetONBMatrix(const Elite::FVector3& worldUp);

	void SetRightHandedProjectionMatrix();
	void SetLeftHandedProjectionMatrix();
	const Elite::FPoint3& GetPosition() const;

	const Elite::FMatrix4& GetInverseViewMatrix() const;
	const Elite::FMatrix4& GetProjectionMatrix() const;

	void SetAspectratio(const float& aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		SetRightHandedProjectionMatrix();
		SetLeftHandedProjectionMatrix();
	};
	void FlipCamera();
#pragma region Movement
	void MoveLocalX(const float deltaTime, const bool positiveDirection);
	void MoveLocalY(const float deltaTime, const bool positiveDirection);
	void MoveLocalZ(const float deltaTime, const bool positiveDirection);

	void RotateAndMoveLRMouse(const float deltaTime, const int x, const int y);
	void RotateMouse(const float deltaTime, const int x, const int y);
	void MoveUpDownMouse(const float deltaTime, const int y);

	void Pan(const float deltaTime, const bool positiveDirection);
	void Tilt(const float deltaTime, const bool positiveDirection);

	void Sprint(bool isSprinting);
#pragma endregion

	const size_t GetID() const;
	const char* GetType() const;
private:
	Elite::FMatrix4 m_ONB; // Look-At Matrix
	Elite::FMatrix4 m_RProjectionMatrix;
	Elite::FMatrix4 m_LProjectionMatrix;
	Elite::FMatrix4 m_InverseONB;

	Elite::FMatrix3 m_RotationMatrix;
	Elite::FPoint3 m_Position;
	Elite::FVector3 m_WorldUp;
	Elite::FVector3 m_U, m_V, m_W;	// Right, Up, Forward

	float m_AspectRatio;
	float m_FOV;
	float m_S;
	float m_ActiveMovementSpeed;
	float m_NearPlane;
	float m_FarPlane;
	bool m_RightHanded = true;

	static const float m_MovementSpeed;
	static const float m_RotationSpeed;
	static const float m_SprintSpeed;
	static size_t m_AmountOfCameras;

	const size_t m_ID;
};

