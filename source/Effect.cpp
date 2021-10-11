#include "pch.h"
#include "Effect.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& effectFile)
	: BaseEffect(pDevice, effectFile)
{
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if ( !m_pDiffuseMapVariable->IsValid() )
	{
		printf("m_pDiffuseMapVariable not valid\n");
	}

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if ( !m_pNormalMapVariable->IsValid() )
	{
		printf("m_pNormalMapVariable not valid\n");
	}

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if ( !m_pSpecularMapVariable->IsValid() )
	{
		printf("m_pSpecularMapVariable not valid\n");
	}

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if ( !m_pGlossinessMapVariable->IsValid() )
	{
		printf("m_pGlossinessMapVariable not valid\n");
	}

	m_pRasterStateIndex = m_pEffect->GetVariableByName("gRasterIdx")->AsScalar();
	if ( !m_pRasterStateIndex->IsValid() )
	{
		printf("m_pRasterStateIndex not valide\n");
	}
}

Effect::~Effect()
{
	if ( m_pRasterStateIndex )
	{
		m_pRasterStateIndex->Release();
	}

	if ( m_pGlossinessMapVariable )
	{
		m_pGlossinessMapVariable->Release();
	}

	if(m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}

	if(m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}

	if ( m_pDiffuseMapVariable )
	{
		m_pDiffuseMapVariable->Release();
	}
}




void Effect::SetTexturMap(ID3D11ShaderResourceView* pResourceView, const TextureResourceType& type)
{
	switch ( type )
	{
		case TextureResourceType::Diffuse:
			if ( m_pDiffuseMapVariable->IsValid() )
				m_pDiffuseMapVariable->SetResource(pResourceView);
			break;
		case TextureResourceType::Normal:
			if ( m_pNormalMapVariable->IsValid() )
				m_pNormalMapVariable->SetResource(pResourceView);
			break;
		case TextureResourceType::Specular:
			if ( m_pSpecularMapVariable->IsValid() )
				m_pSpecularMapVariable->SetResource(pResourceView);
			break;
		case TextureResourceType::Glossiness:
			if ( m_pGlossinessMapVariable->IsValid() )
				m_pGlossinessMapVariable->SetResource(pResourceView);
			break;
		default:
			break;
	}
}
void Effect::SetSamplingMethode(const Technique& tech)
{
	m_pTechnique->Release();
	switch ( tech )
	{
		case Technique::COLOR:
			m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
			printf("\nDefaultTechnique\n\n");
			break;
		case Technique::POINT:
			printf("\nPoint\n\n");
			m_pTechnique = m_pEffect->GetTechniqueByName("Point");
			break;
		case Technique::LINEAR:
			m_pTechnique = m_pEffect->GetTechniqueByName("Linear");
			printf("\nLinear\n\n");
			break;
		case Technique::ANISOTROPIC:
			m_pTechnique = m_pEffect->GetTechniqueByName("Anisotropic");
			printf("\nAnisotropic\n\n");
			break;
		default:
			printf("technique not supported");
			break;
	}
	if ( !m_pTechnique->IsValid() )
	{
		printf("Technique not valid\n");
	}
}

void Effect::SetCullMode(CullMode cullmode)
{
	if ( m_pRasterStateIndex->IsValid() )
	{
		m_pRasterStateIndex->SetInt(unsigned int(cullmode));
	}
}

void Effect::CycleCullMode()
{
	SetCullMode(GetNextCullMode());
}

CullMode Effect::GetNextCullMode()
{
	m_Cullmode = CullMode(( int(Effect::m_Cullmode) + 1 ) % int(CullMode::LAST));
	return m_Cullmode;
}

CullMode Effect::GetCullMode()
{
	return m_Cullmode;
}

const Technique& Effect::GetNextTechnique()
{
	m_Technique = Technique(( int(BaseEffect::m_Technique) + 1 ) % int(Technique::LAST));
	return m_Technique;
}

