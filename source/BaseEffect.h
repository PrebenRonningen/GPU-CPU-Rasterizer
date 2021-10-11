#pragma once
#include <d3dx11effect.h>
#include <string>
#include <sstream>

enum class Technique
{
	COLOR = 0,
	POINT = 1,
	LINEAR = 2,
	ANISOTROPIC = 3,
	LAST = 4
};

enum class TextureResourceType
{
	Diffuse = 0,
	Normal = 1,
	Specular = 2,
	Glossiness = 3,
	Transparency = 4
};

enum class CullMode
{
	NONE = 0,
	BACK = 1,
	FRONT = 2,
	LAST = 3
};

class BaseEffect
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::wstring& effectFile);
	virtual ~BaseEffect();

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& effectFile);
	ID3DX11EffectTechnique* GetTechnique() const;

	ID3DX11EffectMatrixVariable* GetWorldMatrix() const;
	ID3DX11EffectMatrixVariable* GetViewProjectionMatrix() const;
	ID3DX11EffectMatrixVariable* GetViewInverseMatrix() const;

	virtual void CycleCullMode()
	{
	};

	virtual CullMode GetCullMode();
	virtual void CycleSamplingMethode();
	virtual void SetSamplingMethode(const Technique& tech);
	virtual const Technique& GetNextTechnique();

	virtual void SetTexturMap(ID3D11ShaderResourceView* pResourceView, const TextureResourceType& type) = 0;
protected:
	ID3DX11Effect* m_pEffect;

	ID3DX11EffectMatrixVariable* m_pMatViewProjVariable;
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable;
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;

	ID3DX11EffectTechnique* m_pTechnique;

	Technique m_Technique{ Technique::COLOR };
	CullMode m_Cullmode{ CullMode::BACK };
};

