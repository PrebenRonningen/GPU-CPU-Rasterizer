#pragma once
#include <d3dx11effect.h>
#include "Effect.h"
#include "FlatEffect.h"

#include "Object.h"
class Mesh : public Object
{
public:	
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex_Input>& vertexData, const std::vector<uint32_t>& indexData, const std::wstring& effectPath, bool transFlag = false, const Elite::FPoint3& position = Elite::FPoint3(0.0f, 0.0f, 0.0f));
	Mesh(ID3D11Device* pDevice, const char* filePath, const std::wstring& effectPath, bool transFlag = false, const Elite::FPoint3& position = Elite::FPoint3(0.0f, 0.0f, 0.0f), bool flipY = false);
	void EffectSetup(ID3D11Device* pDevice, const std::wstring& effectPath, bool transFlag);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&& other) noexcept = delete;

	virtual bool GetColor(const std::vector<uint32_t>& idx, const Elite::FPoint2& pixel, Elite::RGBColor& pixelColor, float& depth) override;
	void PixelShading(const Vertex_Output& outputVertex, Elite::RGBColor& pixelColor, bool& skipPixel);
	void CalculateNormal(const Vertex_Output& outputVertex, Elite::FVector3& lightDirection, float& observedArea, Elite::RGBColor& pixelColor, Elite::RGBColor& lightColor, float lightIntensity);
	virtual void CalculateBoundingBox(const std::vector<uint32_t>& idx, const uint32_t width, const uint32_t height, Elite::IPoint2& boundingBoxMin, Elite::IPoint2& boundingBoxMax) override;

	virtual const PrimitiveTopology GetTopology() const override;

	void SetPrimitiveTopology(const PrimitiveTopology topology);
	void ParseFile(const char* filePath, bool flipY);
	const std::vector<Vertex_Input>& GetVertexBuffer() const;
	std::vector<Vertex_Output>& GetTransformedVertexBuffer();
	const std::vector<uint32_t>& GetIndexBuffer() const;

	virtual BaseEffect* GetEffect() const;

	virtual void CycleTriangleStripList() override;
	virtual void SetTexture(ID3D11Device* pDevice, const std::string& filePath, const TextureType type) override;
	virtual void ToggleTexture(const TextureType type) override;
	virtual void Render(ID3D11DeviceContext* pDeviceContext) const;
	virtual void Update(const float deltaTime) override;
	virtual const char* GetType() const
	{
		return "Mesh";
	};
	virtual const size_t GetID() const
	{
		return m_ID;
	};
	virtual void CycleCullMode()
	{
		m_pEffect->CycleCullMode();
	};

private:
	void SetTransformedVertexBuffer(std::vector<Vertex_Output>& transformedVertexBuffer, const std::vector<Vertex_Input>& vertexBuffer);
	void SetTransformationMatrix(const Elite::FVector3& worldUp = Elite::FVector3(0.0f, 1.0f, 0.0f));

	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11Buffer* m_pIndexBuffer = nullptr;
	BaseEffect* m_pEffect = nullptr;
	ID3D11InputLayout* m_pVertexLayout = nullptr;

	uint32_t m_AmountIndices = 0;

	bool m_FllipY = false;
	std::vector<Vertex_Input> m_VertexBuffer;
	std::vector<Vertex_Output> m_TransformedVertexBuffer;
	std::vector<uint32_t> m_IndexBuffer;
	std::vector<uint32_t> m_IndexBufferStrip;
	std::vector<uint32_t> m_IndexBufferList;
};

