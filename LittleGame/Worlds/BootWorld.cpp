#include "stdafx.h"
#include "BootWorld.h"
#include "GameFramework/GameFramework.h"

namespace LittleEngine
{
BootWorld::BootWorld() : World("Boot")
{
}

void BootWorld::OnActivated()
{
	m_pLogoFont = Repository()->Load<FontAsset>("Sunscreen.otf");
	if ((m_pLogoDrawer = Game()->UI()->PushContext<UIButtonDrawer>()))
	{
		m_pLogoHeader = m_pLogoDrawer->AddElement<UIElement>("Logo Header");
		m_pLogoHeader->SetText(UIText("Little Engine", 80, Colour::White));
		if (m_pLogoFont)
			m_pLogoHeader->SetFont(*m_pLogoFont);
		m_pLogoHeader->m_transform.UnsetParent();
		m_pLogoHeader->m_transform.nPosition = {0, Fixed(0.8f)};
		m_tokenHandler.AddToken(m_pLogoDrawer->AddButton("Start", std::bind(&BootWorld::OnLoadNextWorld, this)));
		m_tokenHandler.AddToken(m_pLogoDrawer->AddButton("Quit", std::bind(&World::Quit, this)));
		m_pLogoDrawer->SetActive(true);
	}
}

void BootWorld::Tick(Time dt)
{
	World::Tick(dt);

	// Fade logo
	if (m_pLogoDrawer && m_pLogoHeader)
	{
		Fixed seconds = Fixed(GetWorldTime().AsSeconds());
		Fixed speed = 2;
		Fixed alpha = (seconds * speed).Sin().Abs() * 255;
		Colour colour = m_pLogoHeader->GetText()->GetPrimaryColour();
		colour.a = UByte(alpha.ToU32());
		m_pLogoHeader->GetText()->SetPrimaryColour(colour);
	}
}

void BootWorld::OnDeactivating()
{
	m_pLogoDrawer = nullptr;
	m_pLogoHeader = nullptr;
}

void BootWorld::OnLoadNextWorld()
{
	LoadWorld(m_id + 1);
}
} // namespace LittleEngine
