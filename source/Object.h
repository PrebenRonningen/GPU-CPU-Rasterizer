#pragma once
#include "EMath.h"
#include "ERGBColor.h"
#include <vector>
#include "Texture.h"
#include "BaseEffect.h"

struct Vertex_Input
{
	Elite::FPoint3 Position{};
	Elite::FVector3 WorldPosition{ 1, 1, 1 };
	Elite::FVector3 Normal{};
	Elite::FVector3 Tangent{};
	Elite::FVector2 Uv{};
};

struct Vertex_Output
{
	Elite::FPoint4 Position{};
	Elite::FVector4 WorldPosition{};
	Elite::FVector3 Normal{};
	Elite::FVector3 Tangent{};
	Elite::FVector2 Uv{};
	Elite::FVector3 ViewDirection{};
};

enum class PrimitiveTopology
{
	TriangleList = 0,
	TriangleStrip = 1
};

enum class TextureType
{
	Diffuse = 0,
	Normal = 1,
	Specular = 2,
	Glossiness = 3
};

class Object
{
public:
	Object(const Elite::FPoint3& position = Elite::FPoint3(0.0f, 0.0f, 0.0f));
	virtual ~Object()
	{
		delete m_pDiffuseTexture;
		delete m_pNormalTexture;
		delete m_pSpecularTexture;
		delete m_pGlossinessTexture;

		m_pDiffuseTexture = nullptr;
		m_pNormalTexture = nullptr;
		m_pSpecularTexture = nullptr;
		m_pGlossinessTexture = nullptr;
	};

	Object(const Object&) = delete;
	Object(Object&&) noexcept = delete;
	Object& operator=(const Object&) = delete;
	Object& operator=(Object&& other) noexcept = delete;

	virtual bool GetColor(const std::vector<uint32_t>& idx, const Elite::FPoint2& pixel, Elite::RGBColor& pixelColor, float& depth) = 0;
	virtual const PrimitiveTopology GetTopology() const = 0;
	virtual void CalculateBoundingBox(const std::vector<uint32_t>& idx, const uint32_t width, const uint32_t height, Elite::IPoint2& boundingBoxMin, Elite::IPoint2& boundingBoxMax) = 0;

	virtual const std::vector<Vertex_Input>& GetVertexBuffer() const = 0;
	virtual std::vector<Vertex_Output>& GetTransformedVertexBuffer() = 0;
	virtual const std::vector<uint32_t>& GetIndexBuffer() const = 0;
	virtual void CycleTriangleStripList() = 0;

	virtual void Render(ID3D11DeviceContext* pDeviceContext) const = 0;
	virtual void Update(const float deltaTime) = 0;
	virtual void ToggleTexture(const TextureType type) = 0;
	virtual void SetTexture(ID3D11Device* pDevice, const std::string& filePath, const TextureType type) = 0;
	virtual const char* GetType() const
	{
		return "Object";
	};
	virtual const size_t GetID() const
	{
		return m_ID;
	};
	void SetForwardVectpr(const Elite::FVector3& fVector);
	virtual void SetWorldMatrix(const Elite::FVector3& worldUp);
	virtual const Elite::FMatrix4& GetTransformationMatrix() const;
	virtual const Elite::FMatrix4& GetTransposedTransformationMatrix();
	virtual BaseEffect* GetEffect() const = 0;
	virtual void CycleCullMode() = 0;

protected:
	static size_t m_AmountOfObjects;
	bool m_IsEnabled{ true };
	const size_t m_ID;
	PrimitiveTopology m_Topology = PrimitiveTopology::TriangleList;

	Elite::FMatrix4 m_TransformationMatrix;
	Elite::FMatrix4 m_RotationMatrix;

	Elite::FMatrix4 m_TransposedTransformationMatrix;
	Elite::FPoint3 m_Position;
	Elite::FVector3 m_U, m_V, m_W;

	Texture* m_pDiffuseTexture = nullptr;
	Texture* m_pNormalTexture = nullptr;
	Texture* m_pSpecularTexture = nullptr;
	Texture* m_pGlossinessTexture = nullptr;
};
