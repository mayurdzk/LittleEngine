#include "stdafx.h"
#include "KeyboardInput.h"

namespace LittleEngine
{
LiveLine::LiveLine()
{
	liveString.reserve(2048);
	cursorIdx = 0;
}

void LiveLine::Set(String newInput)
{
	liveString = std::move(newInput);
	cursorIdx = liveString.size();
}

void LiveLine::Append(String newInput)
{
	size_t cursorOffset = newInput.size();
	if (!newInput.empty())
	{
		if (cursorIdx == liveString.size())
		{
			liveString += std::move(newInput);
		}
		else
		{
			size_t size2 = liveString.size() - cursorIdx;
			liveString = liveString.substr(0, cursorIdx) + std::move(newInput) + liveString.substr(cursorIdx, size2);
		}
		cursorIdx += cursorOffset;
	}
}

bool LiveLine::Backspace()
{
	if (cursorIdx > 0)
	{
		if (cursorIdx == liveString.size())
		{
			liveString = liveString.substr(0, liveString.size() - 1);
		}
		else
		{
			size_t size2 = liveString.size() - cursorIdx;
			liveString = liveString.substr(0, cursorIdx - 1) + liveString.substr(cursorIdx, size2);
		}
		--cursorIdx;
		return true;
	}
	return false;
}

bool LiveLine::Delete()
{
	if (cursorIdx < liveString.size() && !liveString.empty())
	{
		if (cursorIdx == 0)
		{
			liveString = liveString.substr(1, liveString.size() - 1);
		}
		else
		{
			size_t size2 = liveString.size() - cursorIdx;
			liveString = liveString.substr(0, cursorIdx) + liveString.substr(cursorIdx + 1, size2);
		}
		return true;
	}
	return false;
}

void LiveLine::Clear()
{
	liveString.clear();
	cursorIdx = 0;
}

bool LiveLine::MoveCursor(s8 units)
{
	size_t test = cursorIdx;
	test += units;
	if (test <= liveString.size())
	{
		cursorIdx = test;
		return true;
	}
	return false;
}

void LiveLine::MoveCursorToExtreme(bool bStart)
{
	cursorIdx = bStart ? 0 : liveString.size();
}

Fixed LiveLine::GetCursorNPos() const
{
	return liveString.empty() ? Fixed::One : Fixed(cursorIdx, liveString.size());
}

void KeyboardInput::Update(const LEInput::Frame& frame)
{
	bool bAppendText = false;

	if (frame.IsHeld(KeyCode::LShift) && frame.textInput.ContainsKey(KeyCode::Insert))
	{
		m_liveLine.Append(LEInput::Frame::GetClipboard());
	}

	else if (frame.textInput.ContainsIgnored(KeyCode::Backspace))
	{
		m_liveLine.Backspace();
	}

	else if (frame.textInput.ContainsIgnored(KeyCode::Left))
	{
		m_liveLine.MoveCursor(-1);
	}

	else if (frame.textInput.ContainsIgnored(KeyCode::Right))
	{
		m_liveLine.MoveCursor(1);
	}

	else if (frame.textInput.ContainsIgnored(KeyCode::Home))
	{
		m_liveLine.MoveCursorToExtreme(true);
	}
	
	else if (frame.textInput.ContainsIgnored(KeyCode::End))
	{
		m_liveLine.MoveCursorToExtreme(false);
	}

	else if (frame.textInput.ContainsIgnored(KeyCode::Delete))
	{
		m_liveLine.Delete();
	}

	else if (m_bClearOnEscape && frame.textInput.ContainsIgnored(KeyCode::Escape))
	{
		m_liveLine.Clear();
	}

	/*else if (frame.IsPressed(KeyCode::Backspace))
	{
		m_liveLine.Backspace();
	}

	else if (frame.IsPressed(KeyCode::Left))
	{
		m_liveLine.MoveCursor(-1);
	}

	else if (frame.IsPressed(KeyCode::Right))
	{
		m_liveLine.MoveCursor(1);
	}

	else if (frame.IsPressed(KeyCode::Home))
	{
		m_liveLine.MoveCursorToExtreme(true);
	}

	else if (frame.IsPressed(KeyCode::End))
	{
		m_liveLine.MoveCursorToExtreme(false);
	}

	else if (frame.IsPressed(KeyCode::Delete))
	{
		m_liveLine.Delete();
	}

	else if (m_bClearOnEscape && frame.IsPressed(KeyCode::Escape))
	{
		m_liveLine.Clear();
	}*/

	else
	{
		bAppendText = true;
	}

	if (bAppendText && !frame.textInput.text.empty())
	{
		m_liveLine.Append(frame.textInput.text);
	}
}

void KeyboardInput::Clear()
{
	m_liveLine.Clear();
}

String KeyboardInput::GetLiveString() const
{
	return m_liveLine.liveString;
}
} // namespace LittleEngine