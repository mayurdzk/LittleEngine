#include "stdafx.h"
#include "World.h"
#include "Core/Utils.h"
#include "SFMLAPI/System/Assets.h"
#include "LittleEngine/Audio/LEAudio.h"
#include "LittleEngine/Context/LEContext.h"
#include "LittleEngine/Repository/LERepository.h"
#include "WorldClock.h"
#include "WorldStateMachine.h"
#include "LEGame/Model/GameManager.h"

namespace LittleEngine
{
World::World(String name) : GameObject(std::move(name), "World")
{
	m_uWorldClock = MakeUnique<WorldClock>();
}

World::~World() = default;

bool World::LoadWorld(WorldID id)
{
	Assert(m_pWSM, "WSM is null!");
	return m_pWSM->LoadState(id);
}

void World::Quit()
{
	m_uGame->Quit();
}

Time World::GetWorldTime() const
{
	return m_uWorldClock->GetElapsed();
}

LEInput* World::Input() const
{
	Assert(m_pWSM, "WSM is null!");
	return m_pWSM->m_pContext->Input();
}

void World::OnClearing()
{
}

void World::Tick(Time dt)
{
	PreTick(dt);
	m_uGame->Tick(dt);
	PostTick(dt);
}

void World::Activate()
{
	Assert(m_pWSM, "WSM is null!");
	LOG_D("%s Activated", LogNameStr());
	m_uWorldClock->Restart();
	m_uGame = MakeUnique<GameManager>(*m_pWSM);
	OnActivated();
}

void World::Deactivate()
{
	OnDeactivating();
	m_uGame = nullptr;
	Clear();
	LOG_D("%s Deactivated", LogNameStr());
}

void World::Clear()
{
	OnClearing();
	m_uGame = nullptr;
	m_tokenHandler.Clear();
}
} // namespace LittleEngine
