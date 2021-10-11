#include "pch.h"
#include "SceneGraph.h"
#include "RenderScene.h"

void SceneGraph::AddScene(RenderScene* pNewScene)
{
	auto it = find(m_Scenes.begin(), m_Scenes.end(), pNewScene);
	if ( it == m_Scenes.end() )
	{
		this->m_Scenes.push_back(pNewScene);
		m_pActiveScene = pNewScene;
	}
}

void SceneGraph::RemoveScene(RenderScene* pRemoveScene)
{
	auto it = find(m_Scenes.begin(), m_Scenes.end(), pRemoveScene);

	if ( it != m_Scenes.end() )
	{
		delete* it;
		*it = nullptr;
		m_Scenes.erase(it);

		m_pActiveScene = ( !m_Scenes.empty() ? m_Scenes[0] : nullptr );
	}
}

void SceneGraph::NextScene()
{
	if ( !m_Scenes.empty() )
	{
		auto it = std::find(m_Scenes.begin(), m_Scenes.end(), m_pActiveScene);

		if ( it != m_Scenes.end() )
		{
			m_pActiveScene = ( ++it == m_Scenes.end() ) ? m_Scenes[0] : *it;
		}
	}
}

bool SceneGraph::HasScenes() const
{
	return !m_Scenes.empty();
}

RenderScene* SceneGraph::GetActiveScene() const
{
	return m_pActiveScene;
}

std::vector<RenderScene*> SceneGraph::GetScenes() const
{
	return m_Scenes;
}

void SceneGraph::SetDevice(ID3D11Device* pDevice)
{
	m_pDevice = pDevice;
}

void SceneGraph::Reset()
{
	for ( RenderScene* pScene : m_Scenes )
	{
		delete pScene;
		pScene = nullptr;
	}
	m_Scenes.clear();
	m_pActiveScene = nullptr;
}
