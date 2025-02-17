#include "stdafx.h"
#include "Core/Utils.h"
#include "Tweakable.h"
#if ENABLED(TWEAKABLES)
namespace LittleEngine::Debug
{
Tweakable::Tweakable(String id, TweakType type, String value /* = "" */, void* pTarget /* = nullptr */)
	: m_pTarget(pTarget), m_value(std::move(value)), m_type(type)
{
	static Array<const char*, 4> suffix = {"", "_f32", "_s32", "_b"};
	m_id = std::move(id) + suffix[m_type];
	TweakManager::Instance()->m_tweakables.emplace(m_id, this);
}

Tweakable::~Tweakable()
{
	TweakManager::Instance()->m_tweakables.erase(m_id);
}

void Tweakable::Set(String stringValue)
{
	switch (m_type)
	{
	default:
	case TweakType::STRING:
		SyncString(std::move(stringValue));
		break;

	case TweakType::F32:
		SyncF32(std::move(stringValue));
		break;

	case TweakType::S32:
		SyncS32(std::move(stringValue));
		break;

	case TweakType::BOOL:
		SyncBool(std::move(stringValue));
		break;
	}
	if (m_callback)
	{
		m_callback(m_value);
	}
}

void Tweakable::Bind(void* pVar)
{
	m_pTarget = pVar;
}

void Tweakable::BindCallback(std::function<void(const String&)> callback)
{
	m_callback = std::move(callback);
}

void Tweakable::SyncString(String rawValue)
{
	m_value = std::move(rawValue);
	if (m_pTarget)
	{
		auto pString = reinterpret_cast<String*>(m_pTarget);
		*pString = m_value;
	}
}

void Tweakable::SyncF32(String rawValue)
{
	f32 value = Strings::ToF32(rawValue);
	m_value = Strings::ToString(value);
	if (m_pTarget)
	{
		auto pF32 = reinterpret_cast<f32*>(m_pTarget);
		*pF32 = value;
	}
}

void Tweakable::SyncS32(String rawValue)
{
	s32 value = Strings::ToS32(rawValue);
	m_value = Strings::ToString(value);
	if (m_pTarget)
	{
		auto pS32 = reinterpret_cast<s32*>(m_pTarget);
		*pS32 = value;
	}
}

void Tweakable::SyncBool(String rawValue)
{
	bool bValue = Strings::ToBool(rawValue);
	m_value = Strings::ToString(bValue);
	if (m_pTarget)
	{
		auto pBool = reinterpret_cast<bool*>(m_pTarget);
		*pBool = bValue;
	}
}

TweakManager* TweakManager::Instance()
{
	static TweakManager manager;
	return &manager;
}

Tweakable* TweakManager::Find(const String& id) const
{
	auto iter = m_tweakables.find(id);
	if (iter != m_tweakables.end())
	{
		return iter->second;
	}
	return nullptr;
}
} // namespace LittleEngine::Debug
#endif