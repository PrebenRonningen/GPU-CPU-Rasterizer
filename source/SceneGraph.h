#pragma once
#include "Singleton.h"
#include <vector>

class RenderScene;

class SceneGraph : public Singleton<SceneGraph>
{
public:
	~SceneGraph()
	{
		Reset();
	}

	void AddScene(RenderScene* pNewObject);
	void RemoveScene(RenderScene* pRenderScene);

	void NextScene();

	bool HasScenes() const;
	RenderScene* GetActiveScene() const;
	std::vector<RenderScene*> GetScenes() const;

	void SetDevice(ID3D11Device* pDevice);
	ID3D11Device* m_pDevice = nullptr;
private:
	void Reset() override;

	std::vector<RenderScene*> m_Scenes;
	RenderScene* m_pActiveScene = nullptr;
};