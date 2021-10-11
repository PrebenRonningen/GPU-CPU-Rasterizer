#include "pch.h"
#include "FlatEffect.h"


FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& effectFile)
	: BaseEffect(pDevice, effectFile)
{
	m_pTechnique->Release();
	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if ( !m_pTechnique->IsValid() )
	{
		printf("m_pTechnique not valid\n");
	}

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if ( !m_pDiffuseMapVariable->IsValid() )
	{
		printf("m_pDiffuseMapVariable not valid\n");
	}

	m_pTransparencyMapVariable = m_pEffect->GetVariableByName("gTransparencyMap")->AsShaderResource();
	if ( !m_pTransparencyMapVariable->IsValid() )
	{
		printf("m_pTransparencyMapVariable not valid\n");
	}

}

FlatEffect::~FlatEffect()
{

	if ( m_pTransparencyMapVariable )
	{
		m_pTransparencyMapVariable->Release();
	}

	if ( m_pDiffuseMapVariable )
	{
		m_pDiffuseMapVariable->Release();
	}
}

void FlatEffect::SetTexturMap(ID3D11ShaderResourceView* pResourceView, const TextureResourceType& type)
{
	switch ( type )
	{
		case TextureResourceType::Diffuse:
			if ( m_pDiffuseMapVariable->IsValid() )
				m_pDiffuseMapVariable->SetResource(pResourceView);
			break;
		case TextureResourceType::Transparency:
			if ( m_pTransparencyMapVariable->IsValid() )
				m_pTransparencyMapVariable->SetResource(pResourceView);
			break;
		default:
			printf("TextureResourceType not supported.\n");
			break;
	}
}
