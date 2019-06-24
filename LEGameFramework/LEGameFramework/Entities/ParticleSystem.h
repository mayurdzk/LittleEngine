#pragma once
#include "Core/TRange.h"
#include "LEGameModel/World/Entity.h"
#include "LEGameFramework/Utility/ParticleSystem/PSData.h"
#include "LEGameFramework/Utility/ParticleSystem/PSEmitter.h"

namespace Core
{
class GData;
}

namespace LittleEngine
{
class ParticleSystem : public Entity
{
protected:
	Vec<UPtr<Emitter>> m_emitters;
	bool m_bIsPlaying = false;
#if ENABLED(PROFILER)
	Colour m_profileColour;
#endif

public:
	ParticleSystem();
	~ParticleSystem() override;

	void OnCreated() override;
	void InitParticleSystem(ParticleSystemData data);
	Emitter* GetEmitter(const String& id);

	void Start();
	void Stop();
	inline bool IsPlaying() const
	{
		return m_bIsPlaying;
	}

	void Tick(Time dt) override;
};
} // namespace LittleEngine
