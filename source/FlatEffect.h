#pragma once
#include "BaseEffect.h"
class FlatEffect : public BaseEffect
{
public:
	FlatEffect(ID3D11Device* pDevice, const std::wstring& effectFile);
	~FlatEffect();
	void SetTexturMap(ID3D11ShaderResourceView* pResourceView, const TextureResourceType& type) override;
private:

	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	ID3DX11EffectShaderResourceVariable* m_pTransparencyMapVariable;
};