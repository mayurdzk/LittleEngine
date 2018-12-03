#pragma once
#include "Levels/Level.h"

namespace LittleEngine {
	class LevelManager;
	class AudioManager;

	// \brief Commands as Functors
	class EngineCommand {
	public:
		virtual ~EngineCommand() = default;
		virtual bool operator()() = 0;
	
	protected:
		EngineCommand() = default;
	};

	// \brief Will invoke LevelManager::LoadLevel(levelID) when called
	class LoadLevelCommand : public EngineCommand {
	public:
		LoadLevelCommand(LevelManager& levelManager, const LevelID& levelID);
		virtual bool operator()() override;
	
	private:
		LevelID levelID;
		LevelManager* levelManager;
	};
}
