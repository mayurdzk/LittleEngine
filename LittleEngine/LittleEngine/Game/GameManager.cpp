#include "stdafx.h"
#include "GameManager.h"
#include "LittleEngine/Services/Services.h"
#include "Logger.h"

namespace LittleEngine
{
GameManager::GameManager(World& owner)
	: m_name("GameManager (" + owner.m_name + ")"), m_pWorld(&owner)
{
	m_uUIManager = MakeUnique<UIManager>(*m_pWorld);
	m_uCollisionManager = MakeUnique<CollisionManager>(m_pWorld->m_name);
	Services::ProvideGameManager(*this);
}

GameManager::~GameManager()
{
	Services::UnprovideGameManager(*this);
	for (auto& componentVec : m_uComponents)
	{
		componentVec.clear();
	}
	m_uCollisionManager = nullptr;
	m_uEntities.clear();
	m_uUIManager = nullptr;
}

UIManager* GameManager::UI() const
{
	return m_uUIManager.get();
}

EngineInput* GameManager::Input() const
{
	return Services::Engine()->Input();
}

EngineAudio* GameManager::Audio() const
{
	return Services::Engine()->Audio();
}

EngineRepository* GameManager::Repository() const
{
	return Services::Engine()->Repository();
}

WorldStateMachine* GameManager::Worlds() const
{
	return Services::Engine()->Worlds();
}

CollisionManager* GameManager::Physics() const
{
	return m_uCollisionManager.get();
}

String GameManager::LogName() const
{
	return "[" + m_name + "]";
}

void GameManager::Tick(Time dt)
{
	m_uCollisionManager->Tick(dt);
	for (auto& componentVec : m_uComponents)
	{
		Core::CleanVector<UPtr<Component>>(componentVec, [](UPtr<Component>& uC) { return uC->m_bDestroyed; });
		for (auto& uComponent : componentVec)
		{
			uComponent->Tick(dt);
		}
	}
	Core::CleanVector<UPtr<Entity>>(m_uEntities, [](UPtr<Entity>& uE){ return uE->m_bDestroyed; });
	for (auto& uEntity : m_uEntities)
	{
		uEntity->Tick(dt);
	}
	m_uUIManager->Tick(dt);
}
} // namespace LittleEngine
