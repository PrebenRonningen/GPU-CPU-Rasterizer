#include "pch.h"
#include "Texture.h"

size_t Texture::m_AmountOfTextures = 0;

Texture::Texture( const std::string& filePath)
	: m_pTextureSurface{ IMG_Load(filePath.c_str()) }
	, m_ID{ ++m_AmountOfTextures }
{
	if ( m_pTextureSurface != nullptr ) m_IsSet = true;
}

Texture::Texture(ID3D11Device* pDevice, const std::string& filePath)
	: m_pTextureSurface{ IMG_Load(filePath.c_str()) }
	, m_ID{ ++m_AmountOfTextures }
{
	if ( m_pTextureSurface != nullptr ) m_IsSet = true;

	if ( pDevice == nullptr ) return;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pTextureSurface->w;
	desc.Height = m_pTextureSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_pTextureSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>( m_pTextureSurface->pitch );
	initData.SysMemSlicePitch = static_cast<UINT>( m_pTextureSurface->h * m_pTextureSurface->pitch );

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture2D);
	if ( FAILED(hr) )
		return;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = desc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture2D, &SRVDesc, &m_pTextureResourceView);
	if ( FAILED(hr) )
		return;
}

Texture::~Texture()
{
	m_pTexture2D->Release();
	m_pTextureResourceView->Release();
	SDL_FreeSurface(m_pTextureSurface);
	m_IsSet = false;
}

bool Texture::SetTexture(const std::string& filePath)
{
	if ( m_IsSet )
	{
		SDL_FreeSurface(m_pTextureSurface);
		m_pTextureSurface = IMG_Load(filePath.c_str());
		return true;
	} else
	{
		m_pTextureSurface = IMG_Load(filePath.c_str());
		return true;
	}
}

bool Texture::IsSet() const
{
	return ( this == nullptr ) ? false : m_IsSet;
}

void Texture::ToggleIsSet()
{
	if ( this != nullptr )
	{
		m_IsSet = !m_IsSet;
	}
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView()
{
	return m_pTextureResourceView;
}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv, bool& skip) const
{
	uint32_t* pixels = static_cast<uint32_t*>(m_pTextureSurface->pixels);

	uint32_t idx = (static_cast<uint32_t>(uv.x * m_pTextureSurface->w) + static_cast<uint32_t>(static_cast<uint32_t>(uv.y * m_pTextureSurface->h) * m_pTextureSurface->w)) % static_cast<uint32_t>((m_pTextureSurface->h * m_pTextureSurface->w));
	uint32_t pixel = pixels[idx];

	uint8_t R, G, B, A = 0;
	SDL_GetRGBA(pixel, m_pTextureSurface->format, &R, &G, &B, &A);

	skip = bool(A == 0);

	return Elite::RGBColor(R / 255.f, G / 255.f, B / 255.f);
}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv) const
{
	uint32_t* pixels = static_cast<uint32_t*>(m_pTextureSurface->pixels);

	uint32_t idx = (static_cast<uint32_t>(uv.x * m_pTextureSurface->w) + static_cast<uint32_t>(static_cast<uint32_t>(uv.y * m_pTextureSurface->h) * m_pTextureSurface->w)) % static_cast<uint32_t>((m_pTextureSurface->h * m_pTextureSurface->w));
	uint32_t pixel = pixels[idx];

	uint8_t R, G, B = 0;
	SDL_GetRGB(pixel, m_pTextureSurface->format, &R, &G, &B);


	return Elite::RGBColor(R / 255.f, G / 255.f, B / 255.f);
}