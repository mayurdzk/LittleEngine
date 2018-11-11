#include "le_stdafx.h"
#include "GameClock.h"
#include "Utils.h"

namespace LittleEngine {
	double GameClock::gameTime = 0;

	GameClock::GameClock() {
		startTime = gameTime;
	}

	void GameClock::Restart() {
		startTime = gameTime;
	}

	int64_t GameClock::GetElapsedMicroSeconds() const {
		double elapsedMS = gameTime - startTime;
		return static_cast<int64_t>(elapsedMS * 1000);
	}

	int GameClock::GetElapsedMilliSeconds() const {
		int elapsed = static_cast<int>(gameTime - startTime);
		return elapsed < 0 ? 0 : elapsed;
	}

	int GameClock::GetGameTimeMilliSeconds() {
		return static_cast<int>(gameTime);
	}

	std::string GameClock::ToString(int milliseconds) {
		int secs = milliseconds / 1000;
		milliseconds %= 1000;
		int mins = (secs < 60) ? 0 : secs / 60;
		secs %= 60;
		std::string m = Strings::ToString(mins);
		if (mins < 10) m = "0" + m;
		std::string s = Strings::ToString(secs);
		if (secs < 10) s = "0" + s;
		std::string _m = Strings::ToString(milliseconds);
		if (milliseconds < 10) _m = "00" + _m;
		else if (milliseconds < 100) _m = "0" + _m;
		if (mins == 0) return s + ":" + _m;
		return m + ":" + s + ":" + _m;
	}

	void GameClock::Tick(const Fixed& deltaTime) {
		gameTime += deltaTime.GetDouble();
	}

	void GameClock::Reset() {
		gameTime = 0;
	}
}
