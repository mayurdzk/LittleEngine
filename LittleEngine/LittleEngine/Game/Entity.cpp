#include "stdafx.h"
#include "Logger.h"
#include "Camera.h"
#include "Component.h"
#include "Entity.h"
#include "Camera.h"
#include "LittleEngine/Services/Services.h"

namespace LittleEngine
{
Entity::Entity(String name) : WorldObject(std::move(name), "Entity")
{
	Camera* pWorldCam = Services::WorldCamera();
	if (pWorldCam)
	{
		m_transform.SetParent(pWorldCam->m_transform);
	}
}

Entity::~Entity()
{
	m_pComponents.clear();
}

void Entity::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
	for (auto pComponent : m_pComponents)
	{
		pComponent->SetEnabled(bEnabled);
	}
	LOG_D("%s %s", LogNameStr(), m_bEnabled ? " Enabled" : " Disabled");
}

void Entity::Destruct()
{
	for (auto pComponent : m_pComponents)
	{
		pComponent->Destruct();
	}
	m_bDestroyed = true;
}

void Entity::Tick(Time /*dt*/)
{
	Core::CleanVector<Component*>(m_pComponents,
								  [](Component* pComponent) { return pComponent->m_bDestroyed; });
}
} // namespace LittleEngine
