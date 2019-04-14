#pragma once
#include "Logger.h"
#include "SimpleTime.h"
#include "Version.h"
#include "ComponentTimingType.h"
#include "LittleEngine/Services/IService.h"

namespace LittleEngine
{
class GameManager final : public IService
{
private:
	static constexpr size_t COMPONENT_LINES = ToIdx(TimingType::LAST) + 1;
	static Core::Version s_gameVersion;

private:
	String m_logName;
	Vec<UPtr<class Entity>> m_uEntities;
	Array<Vec<UPtr<class AComponent>>, COMPONENT_LINES> m_uComponents;
	UPtr<class UIManager> m_uUIManager;
	UPtr<class CollisionManager> m_uCollisionManager;
	UPtr<class Camera> m_uWorldCamera;

public:
	static void SetGameVersion(const Core::Version& version);
	static const Core::Version& GetGameVersion();

public:
	GameManager();
	~GameManager() override;

	UIManager* UI() const;
	class EngineInput* Input() const;
	class EngineAudio* Audio() const;
	class EngineRepository* Repository() const;
	class WorldStateMachine* Worlds() const;
	Camera* WorldCamera() const;
	CollisionManager* Physics() const;

public:
	template <typename T>
	T* NewEntity(String name = "Untitled", Vector2 position = Vector2::Zero, Fixed orientation = Fixed::Zero);
	template <typename T>
	T* NewComponent(Entity& owner);

	const char* LogNameStr() const;

private:
	void Tick(Time dt);

	friend class World;
};

template <typename T>
T* GameManager::NewEntity(String name, Vector2 position, Fixed orientation)
{
	static_assert(IsDerived<Entity, T>(), "T must derive from Entity");
	UPtr<T> uT = MakeUnique<T>();
	T* pT = uT.get();
	m_uEntities.emplace_back(std::move(uT));
	pT->OnCreate(std::move(name));
	pT->m_transform.localPosition = position;
	pT->m_transform.localOrientation = orientation;
	LOG_D("%s spawned", pT->LogNameStr());
	return pT;
}

template <typename T>
T* GameManager::NewComponent(Entity& owner)
{
	static_assert(IsDerived<AComponent, T>(), "T must derive from Entity");
	UPtr<T> uT = MakeUnique<T>();
	size_t idx = ToIdx(uT->GetComponentTiming());
	Assert(m_uComponents.size() > idx, "Invalid Component Timing index!");
	auto& componentVec = m_uComponents.at(idx);
	uT->SetOwner(owner);
	T* pT = uT.get();
	componentVec.emplace_back(std::move(uT));
	LOG_D("%s spawned", pT->LogNameStr());
	return pT;
}
} // namespace LittleEngine
