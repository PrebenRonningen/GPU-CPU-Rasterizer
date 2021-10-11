#pragma once
#include <SDL_image.h>
#include <string>
#include "ERGBColor.h"
#include "EMath.h"

class Texture
{
public:
	Texture(ID3D11Device* pDevice, const std::string& filePath);
	Texture(const std::string& filePath);
	~Texture();

	Texture(const Texture&) = delete;
	Texture(Texture&&) noexcept = delete;
	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&& other) noexcept = delete;

	bool SetTexture(const std::string& filePath);
	bool IsSet() const;
	void ToggleIsSet();

	ID3D11ShaderResourceView* GetShaderResourceView();
	Elite::RGBColor Sample(const Elite::FVector2& uv, bool& skip) const;
	Elite::RGBColor Sample(const Elite::FVector2& uv) const;

private:
	ID3D11Texture2D* m_pTexture2D;
	ID3D11ShaderResourceView* m_pTextureResourceView;

	SDL_Surface* m_pTextureSurface;
	static size_t m_AmountOfTextures;
	const size_t m_ID;
	bool m_IsSet = false;
};

