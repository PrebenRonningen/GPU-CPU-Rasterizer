#pragma once
#include <vector>

class Object;
class Camera;
class RenderScene
{
public:
	RenderScene(const char* sceneName);
	~RenderScene();

	void AddObject(Object* pNewObject);
	void RemoveObject(Object* pRemoveObject);
	void NextObject();
	const std::vector<Object*>& GetObjects() const;
	Object* GetActiveObject() const;

	void AddCamera(Camera* pNewCamera);
	void RemoveCamera(Camera* pCamera);
	void NextCamera();
	const std::vector<Camera*>& GetCameras() const;
	bool HasCamera() const;
	Camera* GetActiveCamera() const;

	void ToggleTurnTable();
	bool GetIsTurnTable() const;

	char* GetSceneName() const;

private:
	std::vector<Object*> m_Objects;
	std::vector<Camera*> m_Cameras;
	Object* m_pActiveObject = nullptr;
	Camera* m_pActiveCamera = nullptr;

	char* m_pSceneName;
	bool m_Turntable = false;
};

