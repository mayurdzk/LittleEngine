#pragma once
#include <vector>
#include "SFMLInterface/Input.h"

namespace Game {
	// \brief Virtual gamepad input types
	enum class GameInput {
		Invalid,
		Left,
		Right,
		Up,
		Down,
		Enter,
		Return,
		Select,
		X,
		Y,
		LB,
		RB
	};

	// \brief Virtual gamepad: binds KeyCodes to GameInput
	class Gamepad {
	public:
		GameInput ToGameInput(const KeyState& input) const;
		void Bind(KeyCode keyCode, GameInput key);

	private:
		struct RawInput {
			KeyCode keyCode;

			RawInput(KeyCode keyCode);
			bool operator==(const RawInput& rhs) const;
			bool operator!=(const RawInput& rhs) const;
		};

		struct InputMapping {
			InputMapping(GameInput key);
			InputMapping(GameInput key, KeyCode keyCode);
			void Bind(KeyCode keyCode);
			bool IsMapped(KeyCode keyCode) const;

			GameInput key;
			std::vector<RawInput> rawInputs;
		};

		std::vector<InputMapping> bindings;
	};
}
