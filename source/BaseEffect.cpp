#include "pch.h"
#include "BaseEffect.h"

BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& effectFile)
	: m_Technique{ Technique::COLOR }
{
	m_pEffect = LoadEffect(pDevice, effectFile);

	m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if ( !m_pTechnique->IsValid() )
	{
		printf("m_pTechnique not valid\n");
	}

	m_pMatViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if ( !m_pMatViewProjVariable->IsValid() )
	{
		printf("m_pMatWorldViewProjVariable not valid\n");
	}

	m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if ( !m_pMatViewInverseVariable->IsValid() )
	{
		printf("m_pMatViewInverseVariable not valid\n");
	}

	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if ( !m_pMatWorldVariable->IsValid() )
	{
		printf("m_pMatWorldVariable not valid\n");
	}
}

BaseEffect::~BaseEffect()
{
	if ( m_pMatWorldVariable )
	{
		m_pMatWorldVariable->Release();
	}

	if ( m_pMatViewInverseVariable )
	{
		m_pMatViewInverseVariable->Release();
	}

	if ( m_pMatViewProjVariable )
	{
		m_pMatViewProjVariable->Release();
	}

	if ( m_pTechnique )
	{
		m_pTechnique->Release();
	}

	if ( m_pEffect )
	{
		m_pEffect->Release();
	}
}

ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& effectFile)
{
	HRESULT result = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(effectFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if ( FAILED(result) )
	{
		if ( pErrorBlob != nullptr )
		{
			char* pErrors = (char*) pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for ( unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++ )
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		} else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile! \nPath: " << effectFile;

			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}
void BaseEffect::CycleSamplingMethode()
{
	SetSamplingMethode(GetNextTechnique());
}
void BaseEffect::SetSamplingMethode(const Technique&)
{
}

ID3DX11EffectMatrixVariable* BaseEffect::GetWorldMatrix() const
{
	return m_pMatWorldVariable;
}

ID3DX11EffectMatrixVariable* BaseEffect::GetViewInverseMatrix() const
{
	return m_pMatViewInverseVariable;
}

ID3DX11EffectMatrixVariable* BaseEffect::GetViewProjectionMatrix() const
{
	return m_pMatViewProjVariable;
}


const Technique& BaseEffect::GetNextTechnique()
{
	return m_Technique;
}

CullMode BaseEffect::GetCullMode()
{
	return m_Cullmode;
}

ID3DX11EffectTechnique* BaseEffect::GetTechnique() const
{
	return m_pTechnique;
}

