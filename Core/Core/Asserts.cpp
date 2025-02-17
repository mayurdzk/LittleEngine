#include "stdafx.h"
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include "Asserts.h"
#include "SysDialog.h"
#include "Logger.h"
#if _MSC_VER
#include <Windows.h>
#endif

namespace Core
{
namespace
{
enum class ResponseType
{
	Assert = 0,
	Ignore = 1,
	Disable = 2,
};

std::unordered_set<std::string> disabledIDs;
} // namespace

void AssertWithMsg(bool expr, const char* message, const char* fileName, long lineNumber)
{
	if (expr)
	{
		return;
	}

	std::string assertID = std::string(message) + "_" + fileName + "_" + std::to_string(lineNumber);
	if (disabledIDs.find(assertID) != disabledIDs.end())
	{
		return;
	}
	
	std::string content =
		std::string(message) + "\n[" + fileName + ": " + std::to_string(lineNumber) + "]";
	content += ("\n\nCancel\t: Assert\nTry\t: Ignore\nCont\t: Disable");
	ResponseType response = ResponseType::Assert;

	DialogueData data;
	data.OnButton0 = [&]() { response = ResponseType::Assert; };
	data.OnButton1 = [&]() { response = ResponseType::Ignore; };
	data.OnButton2 = [&]() { response = ResponseType::Disable; };

	data.title = "Assertion Failed!";
	data.content = content;
	CreateSystemDialogue(data);
	switch (response)
	{
	case ResponseType::Assert:
	{
		LOG_E("Assertion failed: %s", message);
#if _MSC_VER
		if (IsDebuggerPresent())
		{
			__debugbreak();	
		}
		else
		{
			assert(false);
		}
#else
		assert(false && message);
#endif
		break;
	}
	case ResponseType::Disable:
	{
		disabledIDs.emplace(assertID);
		break;
	}
	default:
	{
		break;
	}
	}
}
} // namespace Core
