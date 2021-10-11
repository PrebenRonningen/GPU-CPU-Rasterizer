/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include <vector>
#include "Object.h"

struct SDL_Window;
struct SDL_Surface;
class RenderScene;

class Camera;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();
		void Update(const float deltaTime);
		void SetScene(RenderScene* pScene);

		void ToggleRenderMode();


	private:
		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;
		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRendereTargetView;

		void VertexScreenSpaceTransformation(const std::vector<Vertex_Input>& originalVertexBuffer, std::vector<Vertex_Output>& transformedVertexBuffer, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& ViewProjectionMatrix, const Elite::FPoint3& cameraPosition, const uint32_t screenWidth, const uint32_t screenHeight);

		void ClearPixel();
		SDL_Window* m_pWindow = nullptr;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		RenderScene* m_pScene = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		std::vector<float> m_DepthBuffer;
		bool m_DepthRender = false;
		bool m_IsInitialized = false;
		bool m_SoftwareRendereing = false;
	};
}

#endif