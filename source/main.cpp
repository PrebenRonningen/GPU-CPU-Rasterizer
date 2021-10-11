#include "pch.h"
//#undef main
#include "vld.h"
//Standard includes
//#include <iostream>
#include <string>

//Project includes
#include "ETimer.h"
#include "ERenderer.h"

#include "SceneGraph.h"
#include "Camera.h"
#include "RenderScene.h"

#include "Mesh.h"

//Scenes
void SceneOneInitialize(const float& aspectRatio);
void SceneTwoInitialize(const float& aspectRatio);
void SceneThreeInitialize(const float& aspectRatio);
void SceneFourInitialize(const float& aspectRatio);
void SceneFiveInitialize(const float& aspectRatio);

// helpers
void PrintControls();
void Input(float deltaTime);
std::string GetWindowName(std::string windowTitle)
{
	if ( SceneGraph::GetInstance()->HasScenes() )
	{
		if ( SceneGraph::GetInstance()->GetActiveScene()->HasCamera())
		{
			return windowTitle + " - Scene: " + SceneGraph::GetInstance()->GetActiveScene()->GetSceneName() + " - Camera: " + std::to_string(SceneGraph::GetInstance()->GetActiveScene()->GetActiveCamera()->GetID());
		} else
		{
			return windowTitle + " - Scene: " + SceneGraph::GetInstance()->GetActiveScene()->GetSceneName() + " - Camera: NONE";
		}
	} else
	{
		return windowTitle + " - Scene: NONE - Camera: NONE";
	}
}

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);
	std::string windowTitle = "Rasterizer - **Preben Ronningen**";

	const uint32_t width = 1074;
	const uint32_t height = 300;
	SDL_Window* pWindow = SDL_CreateWindow(
		windowTitle.c_str(),
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	Elite::Timer* pTimer = new Elite::Timer();
	Elite::Renderer* pRenderer = new Elite::Renderer(pWindow);
#pragma region Scene Initialization

	float aspectRatio = float(width) / float(height);
	SceneOneInitialize(aspectRatio);

	SceneTwoInitialize(aspectRatio);
	SceneThreeInitialize(aspectRatio);
	SceneFourInitialize(aspectRatio);
	SceneFiveInitialize(aspectRatio);

	pRenderer->SetScene(SceneGraph::GetInstance()->GetActiveScene());
	SDL_SetWindowTitle(pWindow, GetWindowName(windowTitle).c_str());

	//SceneGraph::GetInstance()->NextScene();
	PrintControls();

#pragma endregion

	//Start loop
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;

	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if ( e.key.keysym.scancode == SDL_SCANCODE_V )
				{
					SceneGraph::GetInstance()->NextScene();
					SDL_SetWindowTitle(pWindow, GetWindowName(windowTitle).c_str());
					pRenderer->SetScene(SceneGraph::GetInstance()->GetActiveScene());
					
				}
				if ( e.key.keysym.scancode == SDL_SCANCODE_R )
				{
					pRenderer->ToggleRenderMode();
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_X )
				{
					SceneGraph::GetInstance()->GetActiveScene()->ToggleTurnTable();
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_F )
				{
					for ( Object* pObj : SceneGraph::GetInstance()->GetActiveScene()->GetObjects() )
					{
						pObj->GetEffect()->CycleSamplingMethode();
					}
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_Z )
				{
					SceneGraph::GetInstance()->GetActiveScene()->NextCamera();
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_C )
				{
					for ( Object* pObj : SceneGraph::GetInstance()->GetActiveScene()->GetObjects() )
					{
						pObj->CycleCullMode();
					}
				}
				
				if ( e.key.keysym.scancode == SDL_SCANCODE_T )
				{
					//TODO: Toggle Transparancy On/Off
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_H )
				{
					PrintControls();
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_SPACE )
				{
					SceneGraph::GetInstance()->GetActiveScene()->GetActiveCamera()->ResetFOV();
				}

				if ( e.key.keysym.scancode == SDL_SCANCODE_U || e.key.keysym.scancode == SDL_SCANCODE_I || e.key.keysym.scancode == SDL_SCANCODE_SPACE )
				{
					float fov = std::floorf(SceneGraph::GetInstance()->GetActiveScene()->GetActiveCamera()->GetFOV());
					size_t cameraID = SceneGraph::GetInstance()->GetActiveScene()->GetActiveCamera()->GetID();

					printf("FOV on Camera: %u set to %f\n", unsigned int (cameraID), fov);
				}



				if ( e.key.keysym.scancode == SDL_SCANCODE_1 || e.key.keysym.scancode == SDL_SCANCODE_KP_1 )
				{
					for ( Object* pObj : SceneGraph::GetInstance()->GetActiveScene()->GetObjects() )
					{
						pObj->ToggleTexture(TextureType::Diffuse);
					}
				}
				if ( e.key.keysym.scancode == SDL_SCANCODE_2 || e.key.keysym.scancode == SDL_SCANCODE_KP_2 )
				{
					for ( Object* pObj : SceneGraph::GetInstance()->GetActiveScene()->GetObjects() )
					{
						pObj->ToggleTexture(TextureType::Normal);
					}
				}
				if ( e.key.keysym.scancode == SDL_SCANCODE_3 || e.key.keysym.scancode == SDL_SCANCODE_KP_3 )
				{
					for ( Object* pObj : SceneGraph::GetInstance()->GetActiveScene()->GetObjects() )
					{
						//this falls into a case where spec and gloss both fall into eachother so it doesn't matter if I call Spec or Gloss, they do the same.
						pObj->ToggleTexture(TextureType::Specular);
					}
				}

				break;
			}
		}

		//--------- Input ---------
		Input(pTimer->GetElapsed());
		
		//--------- Update ---------
		pRenderer->Update(pTimer->GetElapsed());

		//--------- Render ---------
		pRenderer->Render();
		
		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = 0.f;
			printf("FPS: %u          \r",pTimer->GetFPS());
		}

	}
	pTimer->Stop();

	delete SceneGraph::GetInstance();
	delete pRenderer;
	delete pTimer;

	//Shutdown "framework"
	ShutDown(pWindow);
	return 0;
}


void SceneOneInitialize(const float& aspectRatio)
{
	RenderScene* pScene = new RenderScene("Scene 1 - Colored Triangle");

	std::vector<Vertex_Input> verteces
	{
		{Elite::FPoint3(0.0f, 0.5f, 0.0f), Elite::FVector3(1.0f, 0.0f, 0.0f)},	// positon, color
		{Elite::FPoint3(0.5f, -0.5f, 0.0f), Elite::FVector3(0.0f, 0.0f, 1.0f)},	// positon, color
		{Elite::FPoint3(-0.5f, -0.5f, 0.0f), Elite::FVector3(0.0f, 1.0f, 0.0f)}	// positon, color
	};

	std::vector<uint32_t> indices{ 0,2,1 };

	Mesh* pMesh = new Mesh(SceneGraph::GetInstance()->m_pDevice, verteces, indices, L"./Resources/PosCol3D.fx", false, Elite::FPoint3(0.0f, 0.0f, 2.0f));
	pScene->AddObject(pMesh);

	Camera* pCamera = new Camera(Elite::FPoint3(0.0f, 0.0f, 10.0f), Elite::FVector3(0.0f, 0.0f, 1.0f), aspectRatio);
	pScene->AddCamera(pCamera);

	SceneGraph::GetInstance()->AddScene(pScene);
}

void SceneTwoInitialize(const float& aspectRatio)
{
	RenderScene* pScene = new RenderScene("Scene 2 - Colored & White Triangle ");

	std::vector<Vertex_Input> verteces0
	{
		{Elite::FPoint3(0.0f, 0.5f, 0.0f), Elite::FVector3(1.0f, 0.0f, 0.0f)},	// positon, color
		{Elite::FPoint3(0.5f, -0.5f, 0.0f), Elite::FVector3(0.0f, 0.0f, 1.0f)},	// positon, color
		{Elite::FPoint3(-0.5f, -0.5f, 0.0f), Elite::FVector3(0.0f, 1.0f, 0.0f)}	// positon, color
	};

	std::vector<Vertex_Input> verteces1
	{
		{Elite::FPoint3(0.0f, 0.25f, 0.5f), Elite::FVector3(1.0f, 1.0f, 1.0f)},		// positon, color
		{Elite::FPoint3(0.25f, -0.25f, 0.5f), Elite::FVector3(1.0f, 1.0f, 1.0f)},	// positon, color
		{Elite::FPoint3(-0.25f, -0.25f, 0.5f), Elite::FVector3(1.0f, 1.0f, 1.0f)}	// positon, color
	};

	std::vector<uint32_t> indices{ 0,2,1 };

	Mesh* pMesh0 = new Mesh(SceneGraph::GetInstance()->m_pDevice, verteces0, indices, L"./Resources/PosCol3D.fx");
	pScene->AddObject(pMesh0);

	Mesh* pMesh1 = new Mesh(SceneGraph::GetInstance()->m_pDevice, verteces1, indices, L"./Resources/PosCol3D.fx");
	pScene->AddObject(pMesh1);

	Camera* pCamera = new Camera(Elite::FPoint3(0.0f, 0.0f, 10.0f), Elite::FVector3(0.0f, 0.0f, 1.0f), aspectRatio);
	pScene->AddCamera(pCamera);

	SceneGraph::GetInstance()->AddScene(pScene);
}

void SceneThreeInitialize(const float& aspectRatio)
{
	RenderScene* pScene = new RenderScene("Scene 3 - Big Triangle");

	std::vector<Vertex_Input> verteces
	{
		{Elite::FPoint3(-3.0f, -3.0f, 0.0f), Elite::FVector3(1.0f, 0.0f, 0.0f)},	// positon, color
		{Elite::FPoint3(0.0f, 3.0f, 0.0f), Elite::FVector3(0.0f, 0.0f, 1.0f)},		// positon, color
		{Elite::FPoint3(3.0f, -3.0f, 0.0f), Elite::FVector3(0.0f, 1.0f, 0.0f)}		// positon, color
	};

	std::vector<uint32_t> indices{ 0,2,1 };

	Mesh* pMesh = new Mesh(SceneGraph::GetInstance()->m_pDevice, verteces, indices, L"./Resources/PosCol3D.fx", false, Elite::FPoint3(0.0f, 0.0f, 2.0f));
	pScene->AddObject(pMesh);

	Camera* pCamera = new Camera(Elite::FPoint3(0.0f, 0.0f, 10.0f), Elite::FVector3(0.0f, 0.0f, 1.0f), aspectRatio);
	pScene->AddCamera(pCamera);

	SceneGraph::GetInstance()->AddScene(pScene);
}

void SceneFourInitialize(const float& aspectRatio)
{
	RenderScene* pScene = new RenderScene("Scene 4 - Textured Quad");

	std::vector<Vertex_Input> verteces
	{ 
		{Elite::FPoint3(-3.0f, 3.0f, 0.0f),	Elite::FVector3(1.0f, 0.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.0f, 0.0f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(0.0f, 3.0f, 0.0f),	Elite::FVector3(0.0f, 1.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.5f, 0.0f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(3.0f, 3.0f, 0.0f),	Elite::FVector3(0.0f, 0.0f, 1.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{1.0f, 0.0f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(-3.0f, 0.0f, 0.0f),	Elite::FVector3(1.0f, 0.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.0f, 0.5f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(0.0f, 0.0f, 0.0f),	Elite::FVector3(0.0f, 1.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.5f, 0.5f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(3.0f, 0.0f, 0.0f),	Elite::FVector3(0.0f, 0.0f, 1.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{1.0f, 0.5f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(-3.0f, -3.0f, 0.0f),Elite::FVector3(1.0f, 0.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.0f, 1.0f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(0.0f, -3.0f, 0.0f),	Elite::FVector3(0.0f, 1.0f, 0.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{0.5f, 1.0f}},	// positon, color, normal, tangent, UV
		{Elite::FPoint3(3.0f, -3.0f, 0.0f),	Elite::FVector3(0.0f, 0.0f, 1.0f), Elite::FVector3(), Elite::FVector3(), Elite::FVector2{1.0f, 1.0f}}	// positon, color, normal, tangent, UV
	};

	std::vector<uint32_t> indices{ 0,3,1,	3,4,1,	1,4,2,	4,5,2,	3,6,4,	6,7,4,	4,7,5,	7,8,5 };

	Mesh* pMesh = new Mesh(SceneGraph::GetInstance()->m_pDevice, verteces, indices, L"./Resources/PosCol3D.fx", false, Elite::FPoint3(0.0f, 0.0f, 2.0f));

	pMesh->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/uv_grid_2.png", TextureType::Diffuse);
	pMesh->GetEffect()->CycleSamplingMethode();
	pScene->AddObject(pMesh);

	Camera* pCamera = new Camera(Elite::FPoint3(0.0f, 0.0f, 10.0f), Elite::FVector3(0.0f, 0.0f, 1.0f), aspectRatio);
	pScene->AddCamera(pCamera);

	SceneGraph::GetInstance()->AddScene(pScene);
}

void SceneFiveInitialize(const float& aspectRatio)
{
	RenderScene* pScene = new RenderScene("Scene 5 - Vehicle & Transparancy");

	Mesh* pMesh0 = new Mesh(SceneGraph::GetInstance()->m_pDevice, "./Resources/vehicle.obj", L"./Resources/PosCol3D.fx", false, Elite::FPoint3(0.0f, 0.0f, 20.0f), true);
	pMesh0->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/vehicle_diffuse.png", TextureType::Diffuse);
	pMesh0->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/vehicle_normal.png", TextureType::Normal);
	pMesh0->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/vehicle_specular.png", TextureType::Specular);
	pMesh0->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/vehicle_gloss.png", TextureType::Glossiness);
	pMesh0->GetEffect()->CycleSamplingMethode();
	pScene->AddObject(pMesh0);


	Mesh* pMesh1 = new Mesh(SceneGraph::GetInstance()->m_pDevice, "./Resources/fireFX.obj", L"./Resources/Trans3D.fx", true, Elite::FPoint3(0.0f, 0.0f, 20.0f), true);
	pMesh1->SetTexture(SceneGraph::GetInstance()->m_pDevice, "./Resources/fireFX_diffuse.png", TextureType::Diffuse);
	pScene->AddObject(pMesh1);

	Camera* pCamera = new Camera(Elite::FPoint3(0.0f, 0.0f, 70.0f), Elite::FVector3(0.0f, 0.0f, 1.0f), aspectRatio);
	pMesh1->GetEffect()->CycleSamplingMethode();
	pScene->AddCamera(pCamera);

	SceneGraph::GetInstance()->AddScene(pScene);
}

void Input(float deltaTime)
{
	int x{}, y{};
	const Uint8* keyState = SDL_GetKeyboardState(NULL);

	bool hasNoScene = !SceneGraph::GetInstance()->HasScenes();
	bool sceneHasNoCamera = !SceneGraph::GetInstance()->GetActiveScene()->HasCamera();

	if ( hasNoScene || sceneHasNoCamera )
	{
		return;
	}

	Camera* pActiveCamera = SceneGraph::GetInstance()->GetActiveScene()->GetActiveCamera();
	float fov = pActiveCamera->GetFOV();
	const float cameraDeltaFOV = 25.0f;
	
	( keyState[SDL_SCANCODE_LSHIFT] ) ? pActiveCamera->Sprint(true) : pActiveCamera->Sprint(false);


	if ( keyState[SDL_SCANCODE_A] )
	{
		pActiveCamera->MoveLocalX(deltaTime, false);
	}
	if ( keyState[SDL_SCANCODE_Q] )
	{
		pActiveCamera->MoveLocalY(deltaTime, false);
	}
	if ( keyState[SDL_SCANCODE_W] )
	{
		pActiveCamera->MoveLocalZ(deltaTime, false);
	}
	if ( keyState[SDL_SCANCODE_D] )
	{
		pActiveCamera->MoveLocalX(deltaTime, true);
	}
	if ( keyState[SDL_SCANCODE_E] )
	{
		pActiveCamera->MoveLocalY(deltaTime, true);
	}
	if ( keyState[SDL_SCANCODE_S] )
	{
		pActiveCamera->MoveLocalZ(deltaTime, true);
	}

	if ( keyState[SDL_SCANCODE_LEFT] )
	{
		pActiveCamera->Pan(deltaTime, true);
	}
	if ( keyState[SDL_SCANCODE_RIGHT] )
	{
		pActiveCamera->Pan(deltaTime, false);
	}
	if ( keyState[SDL_SCANCODE_DOWN] )
	{
		pActiveCamera->Tilt(deltaTime, false);
	}
	if ( keyState[SDL_SCANCODE_UP] )
	{
		pActiveCamera->Tilt(deltaTime, true);
	}

	if ( keyState[SDL_SCANCODE_I] )
	{
		pActiveCamera->SetFOV(fov + cameraDeltaFOV * deltaTime);
	}

	if ( keyState[SDL_SCANCODE_U] )
	{
		pActiveCamera->SetFOV(fov - cameraDeltaFOV * deltaTime);
	}

	if ( SDL_GetRelativeMouseState(&x, &y) == NULL ) return;

	if ( SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1) && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3) )
	{
		pActiveCamera->MoveUpDownMouse(deltaTime, y);
	} else
	{
		if ( SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1) )
		{
			pActiveCamera->RotateAndMoveLRMouse(deltaTime, x, y);

		}
		if ( SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3) )
		{
			pActiveCamera->RotateMouse(deltaTime, x, y);
		}
	}
}

void PrintControls()
{
	//---------- CONTROLS ----------
	printf("||----------- CAMERA CONTROLS -----------\n");
	printf("|| Move --------------------------------- WASDQE\n");
	printf("|| Pan/Tilt ----------------------------- Arrows\n");
	
	printf("|| Move --------------------------------- LMB+Drag\n");
	printf("|| Move UP/DOWN-------------------------- LMB+RMB+Drag\n");
	printf("|| Pan/Tilt ----------------------------- RMB+Drag\n");

	printf("|| Change FOV --------------------------- I/U\n");
	printf("|| Reset FOV ---------------------------- SPACE\n||\n");

	printf("||--------------- SETTINGS --------------\n");
	printf("|| Toggle RenderMode -------------------- R\n");
	printf("|| Cycle SampelingMode ------------------ F\n");
	//printf("|| Toggle Transparancy ------------------ T\n");
	printf("|| Cycle CullMode ----------------------- C\n\n");
	printf("|| Toggle TurnTable --------------------- X\n");
	printf("|| Switch Scene ------------------------- V\n");	// only if there's more than 1 scene.
	printf("|| Switch Camera ------------------------ Z\n\n");	// only if there's more than 1 camera in a scene.
	printf("|| Toggle Diffuse Texture on/off -------- 1 or keypad-1 - (Software Only)\n");
	printf("|| Toggle Normal Texture on/off --------- 2 or keypad-2 - (Software Only)\n");
	printf("|| Toggle Spec & Gloss Texture on/off --- 3 or keypad-3 - (Software Only)\n||\n");

	printf("||----------------- HELP ---------------- \n");
	printf("|| Show Controls ------------------------ H\n||\n");
	printf("||--------------- CONTROLS --------------\n");
}