#include "stdafx.h"
#include "WorldObject.h"
#include "LittleEngine/Services/Services.h"
#include "LittleEngine/Game/GameManager.h"

namespace LittleEngine
{
WorldObject::WorldObject(const String& name, const String& baseClass) : Inheritable(name, baseClass)
{
}

WorldObject::~WorldObject() = default;

void WorldObject::BindInput(const EngineInput::Delegate& Callback)
{
	m_tokenHandler.AddToken(Services::Game()->Input()->Register(Callback));
}
} // namespace LittleEngine
