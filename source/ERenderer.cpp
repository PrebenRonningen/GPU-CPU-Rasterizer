#include "pch.h"
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "ERenderer.h"
#include "ERGBColor.h"
#include "SceneGraph.h"
#include "RenderScene.h"

#include "Camera.h"
#include "Object.h"
#include "Mesh.h"

Elite::Renderer::Renderer(SDL_Window * pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);

	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);

	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*) m_pBackBuffer->pixels;

	//Initialize DirectX pipeline
	if ( SUCCEEDED(InitializeDirectX()))
	{
		m_IsInitialized = true;
		printf("DirectX is ready\n");
	}
	SceneGraph::GetInstance()->SetDevice(m_pDevice);

	m_DepthBuffer.resize(static_cast<uint64_t>( m_Height ) * m_Width, FLT_MAX);
}

void Elite::Renderer::SetScene(RenderScene* pScene)
{
	m_pScene = pScene;
}

void Elite::Renderer::ToggleRenderMode()
{
	m_SoftwareRendereing = !m_SoftwareRendereing;
	for ( RenderScene* pScene: SceneGraph::GetInstance()->GetScenes())
	{
		for ( Camera* pCam : pScene->GetCameras() )
		{
			pCam->FlipCamera();
		}
	}
}

Elite::Renderer::~Renderer()
{
	if ( m_pRendereTargetView )
	{
		m_pRendereTargetView->Release();
	}

	if ( m_pRenderTargetBuffer )
	{
		m_pRenderTargetBuffer->Release();
	}

	if ( m_pDepthStencilView )
	{
		m_pDepthStencilView->Release();
	}

	if ( m_pDepthStencilBuffer )
	{
		m_pDepthStencilBuffer->Release();
	}

	if ( m_pSwapChain )
	{
		m_pSwapChain->Release();
	}

	if ( m_pDeviceContext )
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	if ( m_pDevice )
	{
		m_pDevice->Release();
	}

	if ( m_pDXGIFactory )
	{
		m_pDXGIFactory->Release();
	}
}

void Elite::Renderer::Render()
{
	if ( !m_SoftwareRendereing )
	{
		if ( !m_IsInitialized )
		{
			return;
		}

		//Clear Buffer
		Elite::RGBColor clearColor = Elite::RGBColor(0.231f, 0.235f, 0.33f);
		m_pDeviceContext->ClearRenderTargetView(m_pRendereTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		//Render
		//...
		if ( m_pScene != nullptr && m_pScene->HasCamera() )
		{
			Elite::FMatrix4 worldViewProjectionMatrix = m_pScene->GetActiveCamera()->GetProjectionMatrix() * m_pScene->GetActiveCamera()->GetInverseViewMatrix();
			Elite::FMatrix4 viewInverse = m_pScene->GetActiveCamera()->GetInverseViewMatrix();;
			Elite::FMatrix4 worldMatrix = Elite::FMatrix4::Identity();
			
			for ( Object* pObj : m_pScene->GetObjects() )
			{
				worldMatrix = pObj->GetTransposedTransformationMatrix();
				pObj->GetEffect()->GetWorldMatrix()->SetMatrix(&worldMatrix[0][0]);
				pObj->GetEffect()->GetViewProjectionMatrix()->SetMatrix(&worldViewProjectionMatrix[0][0]);
				pObj->GetEffect()->GetViewInverseMatrix()->SetMatrix(&viewInverse[0][0]);
				pObj->Render(m_pDeviceContext);
			}
		}
		//Present
		m_pSwapChain->Present(0, 0);
	} else
	{
		SDL_LockSurface(m_pBackBuffer);
		Elite::RGBColor finalColor{};
		Elite::IPoint2 pixel{};

		m_DepthBuffer.assign(m_DepthBuffer.size(), FLT_MAX);

		std::vector<uint32_t> idx(3);

		const Elite::FMatrix4 ViewProjectionMatrix = m_pScene->GetActiveCamera()->GetProjectionMatrix() * m_pScene->GetActiveCamera()->GetInverseViewMatrix();

		const Camera* pCamera = m_pScene->GetActiveCamera();
		//Loop over all the pixels
		ClearPixel();

		//InputData from 3DMesh
		for ( Object* pObj : m_pScene->GetObjects() )
		{
			//InputData from 
			//ProjectionStage 
			VertexScreenSpaceTransformation(pObj->GetVertexBuffer(), pObj->GetTransformedVertexBuffer(), pObj->GetTransformationMatrix(), ViewProjectionMatrix, Elite::FPoint3(pCamera->GetInverseViewMatrix()[3].xyz), m_Width, m_Height);
			//ProjectionStage
			bool isTriangleStrip = bool(pObj->GetTopology());

			for ( uint32_t i{}; i <= pObj->GetIndexBuffer().size() - 3; ( isTriangleStrip ) ? i++ : i += 3 )
			{
				idx[0] = pObj->GetIndexBuffer()[i];
				idx[1] = pObj->GetIndexBuffer()[( isTriangleStrip ) ? ( ( i % 2 == 0 ) ? i + 1 : i + 2 ) : ( i + 1 )];
				idx[2] = pObj->GetIndexBuffer()[( isTriangleStrip ) ? ( ( i % 2 == 0 ) ? i + 2 : i + 1 ) : ( i + 2 )];

				if ( idx[0] == idx[1] || idx[0] == idx[2] || idx[1] == idx[2] )
				{
					continue;
				}

				//OptimizationStage
				Elite::IPoint2 boundingBoxMin{ INT_MAX, INT_MAX };
				Elite::IPoint2 boundingBoxMax{ 0,0 };

				pObj->CalculateBoundingBox(idx, m_Width, m_Height, boundingBoxMin, boundingBoxMax);
				//OptimizationStage

				for ( uint32_t r = boundingBoxMin.y; r < uint32_t(boundingBoxMax.y); r++ )
				{
					pixel.y = r;
					for ( uint32_t c = boundingBoxMin.x; c < uint32_t(boundingBoxMax.x); c++ )
					{
						finalColor *= 0;
						pixel.x = c;

						//RasterStage
						if ( pObj->GetColor(idx, pixel, finalColor, m_DepthBuffer[static_cast<uint64_t>( c ) + ( static_cast<uint64_t>( r ) * m_Width )]) )
						{
							finalColor.MaxToOne();

							if ( m_DepthRender )
							{
								// I liked it at 0.99f mor than 0.985f
								float depthColor = Remap(m_DepthBuffer[static_cast<uint64_t>( c ) + static_cast<uint64_t>( r ) * m_Width], 0.995f, 1.0f);
								depthColor = ( depthColor < 0.f ) ? 0.f : depthColor;
								depthColor = ( depthColor > 1.f ) ? 1.f : depthColor;

								finalColor.r = finalColor.g = finalColor.b = depthColor;
								finalColor.MaxToOne();
							}
							finalColor *= 255;

							m_pBackBufferPixels[c + ( r * m_Width )] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>( finalColor.r ),
								static_cast<uint8_t>( finalColor.g ),
								static_cast<uint8_t>( finalColor.b ));
						}
					}
				}
			}
		}
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}
}

void Elite::Renderer::Update(const float deltaTime)
{
	if ( m_pScene->GetIsTurnTable() )
	{
		for ( Object* pObj : m_pScene->GetObjects() )
		{
			pObj->Update(deltaTime);
		}
	}
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlag = 0;

#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT	result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlag, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);
	if ( FAILED(result) )
	{
		printf("[WARNING] Device creation: FAILED");
		return result;
	}

	//Create DXGI Factory to create SwapChain based on hardware
	result = CreateDXGIFactory(__uuidof( IDXGIFactory ), reinterpret_cast<void**>( &m_pDXGIFactory ));
	if ( FAILED(result) )
	{
		printf("[WARNING] DXGIFactory creation: FAILED");
		return result;
	}

	//Create SwapChain Descripto
	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

	//Create SwapChain and hook ot onto the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
	if ( FAILED(result) )
	{
		printf("[WARNING] Swap-Chain creation: FAILED");
		return result;
	}

	//Create the depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if ( FAILED(result) )
	{
		printf("[WARNING] Texture2D creation: FAILED");
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if ( FAILED(result) )
	{
		printf("[WARNING] DepthStencilView creation: FAILED");
		return result;
	}

	//Create rhe RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof( ID3D11Texture2D ), reinterpret_cast<void**>( &m_pRenderTargetBuffer ));
	if ( FAILED(result) )
	{
		printf("[WARNING] GetBuffer: FAILED");
		return result;
	}
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRendereTargetView);
	if ( FAILED(result) )
	{
		printf("[WARNING] RenderTargetView creation: FAILED");
		return result;
	}

	//Bind the View to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRendereTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>( m_Width );
	viewPort.Height = static_cast<float>( m_Height );
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}

void Elite::Renderer::ClearPixel()
{
	for ( uint32_t r = 0; r < m_Height; ++r )
	{
		for ( uint32_t c = 0; c < m_Width; ++c )
		{
			m_pBackBufferPixels[c + ( r * m_Width )] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>( 50 ),
				static_cast<uint8_t>( 50 ),
				static_cast<uint8_t>( 50 ));
		}
	}
}

void Elite::Renderer::VertexScreenSpaceTransformation(const std::vector<Vertex_Input>& originalVertexBuffer, std::vector<Vertex_Output>& transformedVertexBuffer, const Elite::FMatrix4& worldMatrix, const Elite::FMatrix4& ViewProjectionMatrix, const Elite::FPoint3& cameraPosition, const uint32_t screenWidth, const uint32_t screenHeight)
{
	for ( int i{}; i < originalVertexBuffer.size(); i++ )
	{
		Elite::FMatrix4 worldViewProj = ViewProjectionMatrix * worldMatrix;
		transformedVertexBuffer[i].Position = worldViewProj * Elite::FPoint4(originalVertexBuffer[i].Position, 1);
		transformedVertexBuffer[i].Normal = ( Inverse(worldMatrix) * Elite::FVector4(originalVertexBuffer[i].Normal, 1) ).xyz;
		transformedVertexBuffer[i].ViewDirection = Elite::GetNormalized(Elite::FMatrix4(worldMatrix) * Elite::FVector4(Elite::FPoint3(originalVertexBuffer[i].WorldPosition) - cameraPosition)).xyz;
		

		transformedVertexBuffer[i].Position.x /= transformedVertexBuffer[i].Position.w;
		transformedVertexBuffer[i].Position.y /= transformedVertexBuffer[i].Position.w;
		transformedVertexBuffer[i].Position.z /= transformedVertexBuffer[i].Position.w;

		transformedVertexBuffer[i].Position.x = ( ( transformedVertexBuffer[i].Position.x + 1 ) / 2.0f ) * screenWidth;
		transformedVertexBuffer[i].Position.y = ( ( 1 - transformedVertexBuffer[i].Position.y ) / 2.0f ) * screenHeight;

		transformedVertexBuffer[i].Tangent = ( Inverse(worldMatrix) * Elite::FVector4(originalVertexBuffer[i].Tangent, 1) ).xyz;
	}
}