#include "stdafx.h"
#include "SFML/Window/Clipboard.hpp"
#include "InputDataFrame.h"

namespace LittleEngine
{
KeyState::KeyState(s32 keyType, const char* szName)
	: keyType(keyType), szName(szName), bPressed(false)
{
}

KeyType KeyState::GetKeyType() const
{
	return static_cast<KeyType>(keyType);
}

const char* KeyState::GetNameStr() const
{
	return szName;
}

bool TextInput::ContainsChar(char c) const
{
	size_t idx = text.find(c);
	return idx != String::npos;
}

bool TextInput::ContainsKey(s32 keyCode) const
{
	return Core::Search(keys, static_cast<KeyType>(keyCode)) != keys.end();
}

bool TextInput::ContainsIgnored(KeyCode keyCode) const
{
	return ignoredChars.find(keyCode) != ignoredChars.end();
}

void TextInput::Reset()
{
	text.clear();
	keys.clear();
	ignoredChars.clear();
}

String InputDataFrame::GetClipboard()
{
	return sf::Clipboard::getString();
}
} // namespace LittleEngine