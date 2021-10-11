
#pragma once
#include "BaseEffect.h"

class Effect : public BaseEffect
{
public:
	Effect(ID3D11Device* pDevice, const std::wstring& effectFile);
	~Effect();
	
	void SetTexturMap(ID3D11ShaderResourceView* pResourceView, const TextureResourceType& type) override;
	void SetSamplingMethode(const Technique& technique) override;

	void SetCullMode(CullMode cullmode);
	void CycleCullMode() override;
	CullMode GetCullMode() override;
	const Technique& GetNextTechnique() override;
private:
	CullMode GetNextCullMode();

	ID3DX11EffectScalarVariable* m_pRasterStateIndex;

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
};