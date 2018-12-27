#pragma once
#include "SFMLInterface/SystemClock.h"
#include "Engine/CoreGame.hpp"

namespace LittleEngine {
	// \brief Use to measure game time
	// Note: This DOES NOT run on its own, requires Engine to call Tick(deltaMS)!
	class GameClock {
	private:
		// Base game time maintained by Engine
		static double gameTime;
		// An instance will use this offset to calculate its current time
		double m_startTime;

	public:
		GameClock();

		void Restart();
		int64_t GetElapsedMicroSeconds() const;
		int GetElapsedMilliSeconds() const;
		
		static int GetGameTimeMilliSeconds();
		static std::string ToString(int milliseconds);

	private:
		// Global Ticks to be provided by Engine
		static void Tick(const Fixed& deltaMS);
		// On App Reload etc
		static void Reset();

		friend class Engine;
	};
}
