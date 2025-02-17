#include "stdafx.h"
#include "Core/Logger.h"
#include "SFMLAPI/Rendering/Primitives/SFRect.h"
#include "LittleEngine/Context/LEContext.h"
#include "UIContext.h"
#include "UIWidget.h"
#include "UIWidgetMatrix.h"
#include "LEGame/Model/GameManager.h"
#include "LEGame/Model/World/World.h"
#include "LEGame/Model/UI/UIElement.h"

namespace LittleEngine
{
UIContext::UIContext() = default;

UIContext::~UIContext()
{
	SetActive(false);
	m_uUIWidgets->Clear();
	m_uiElements.clear();
}

void UIContext::OnCreate(String name, LayerID rootLayer)
{
	SetNameAndType(std::move(name), "UIContext");
	m_uUIWidgets = MakeUnique<UIWidgetMatrix>();
	s32 layerDelta = rootLayer - LAYER_UI;
	m_pRoot = AddElement<UIElement>(String(GetNameStr()) + "_Root", nullptr, layerDelta);
	OnCreated();
	SetActive(true);
}

LayerID UIContext::GetMaxLayer() const
{
	LayerID maxLayer = m_pRoot->m_layer;
	for (const auto& uElement : m_uiElements)
	{
		if (uElement->m_layer > maxLayer)
		{
			maxLayer = uElement->m_layer;
		}
	}
	m_uUIWidgets->ForEach([&maxLayer](const UPtr<UIWidget>& uWidget) {
		if (uWidget->GetMaxLayer() > maxLayer)
		{
			maxLayer = uWidget->GetMaxLayer();
		}
	});
	return maxLayer;
}

void UIContext::SetActive(bool bActive, bool bResetSelection)
{
	Assert(g_pGameManager, "GameManager is null!");
	m_inputTokens.clear();
	if (bActive)
	{
		if (bResetSelection)
		{
			ResetSelection();
		}
		m_inputTokens.push_back(g_pGameManager->Input()->Register(
			[&](const LEInput::Frame& frame) -> bool { return OnInput(frame); }, true));
	}
}

void UIContext::ResetSelection()
{
	m_uUIWidgets->Reset(true);
	m_uUIWidgets->ForEach([](UPtr<UIWidget>& uWidget) { uWidget->Deselect(); });
	UIWidget* pSelected = GetSelected();
	if (pSelected)
	{
		pSelected->Select();
	}
}

UIWidget* UIContext::GetSelected()
{
	return m_uUIWidgets->Current();
}

UIElement* UIContext::GetRoot() const
{
	return m_pRoot;
}

UIContext::OnCancelled::Token UIContext::SetOnCancelled(OnCancelled::Callback callback, bool bAutoDestroy)
{
	m_bAutoDestroyOnCancel = bAutoDestroy;
	return m_onCancelledDelegate.Register(std::move(callback));
}

void UIContext::Destruct()
{
	OnDestroying();
	m_bDestroyed = true;
}

void UIContext::Tick(Time dt)
{
	if (m_bDestroyed)
	{
		return;
	}
	for (auto& uElement : m_uiElements)
	{
		uElement->Tick(dt);
	}
	bool bPointerSelection = false;
	const MouseInput& pointerState = g_pGameManager->Input()->GetMouseState();
	if (pointerState.bInViewport)
	{
		UIWidget* pSelected = GetSelected();
		m_uUIWidgets->ForEach([this, &pSelected, &bPointerSelection, &pointerState](UIContext::UUIWidget& uUIWidget) {
			Vector2 mp = pointerState.worldPosition;
			if (!bPointerSelection && uUIWidget->IsPointInBounds(mp))
			{
				bPointerSelection = true;
				auto pToActivate = uUIWidget.get();
				if (pSelected != pToActivate)
				{
					if (m_uUIWidgets->Select(pToActivate))
					{
						if (pSelected)
						{
							pSelected->Deselect();
						}
						pToActivate->Select();
					}
					else
					{
						bPointerSelection = false;
						LOG_E("%s ERROR! %s not found in widget matrix!", m_logName.c_str(),
							  pToActivate->m_logName.c_str());
					}
				}
			}
		});
	}

	if (bPointerSelection)
	{
		// Pointer is inside a widget's bounds
		m_pPointerOver = GetSelected();
		if (m_pPointerOver)
		{
			if (m_mbState.bEnterPressed)
			{
				OnEnterPressed();
			}
			else if (m_mbState.bEnterReleased)
			{
				OnEnterReleased(m_bInteracting);
			}
		}
	}
	else
	{
		if (m_bInteracting && m_pPointerOver)
		{
			// Pointer has moved out
			OnEnterReleased(false);
		}
		m_pPointerOver = nullptr;
	}

	m_uUIWidgets->ForEach([dt](UIContext::UUIWidget& uUIWidget) { uUIWidget->Tick(dt); });
}

void UIContext::OnCreated()
{
}

void UIContext::OnDestroying()
{
}

bool UIContext::OnInput(const LEInput::Frame& frame)
{
	if (m_bDestroyed)
	{
		return false;
	}

	m_mbState.bEnterPressed = frame.IsPressed(KeyType::MOUSE_BTN_0);
	m_mbState.bEnterReleased = frame.IsReleased(KeyType::MOUSE_BTN_0);
	
	if (frame.IsPressed({KeyCode::Enter, KeyType::JOY_BTN_0}))
	{
		OnEnterPressed();
	}
	if (frame.IsReleased({KeyCode::Enter, KeyType::JOY_BTN_0}))
	{
		OnEnterReleased(m_bInteracting);
	}
	if (frame.IsReleased({KeyCode::Escape, KeyType::JOY_BTN_1}))
	{
		OnBackReleased();
	}
	if (frame.IsReleased(KeyCode::Up))
	{
		OnUp();
	}
	if (frame.IsReleased(KeyCode::Down))
	{
		OnDown();
	}
	if (frame.IsReleased(KeyCode::Left))
	{
		OnLeft();
	}
	if (frame.IsReleased(KeyCode::Right))
	{
		OnRight();
	}
	return true;
}

void UIContext::OnUp()
{
	if (m_bInteracting || m_uUIWidgets->CurrentVecCount() < 2)
	{
		return;
	}
	UIWidget* pSelected = GetSelected();
	if (pSelected)
	{
		pSelected->Deselect();
	}
	pSelected = m_uUIWidgets->NextSelectableVertical(false);
	if (pSelected)
	{
		pSelected->Select();
	}
}

void UIContext::OnDown()
{
	if (m_bInteracting || m_uUIWidgets->CurrentVecCount() < 2)
	{
		return;
	}
	UIWidget* pWidget = GetSelected();
	if (pWidget)
	{
		pWidget->Deselect();
	}
	pWidget = m_uUIWidgets->NextSelectableVertical(true);
	if (pWidget)
	{
		pWidget->Select();
	}
}

void UIContext::OnLeft()
{
	if (m_bInteracting || m_uUIWidgets->NumColumns() < 2)
	{
		return;
	}
	UIWidget* pWidget = GetSelected();
	if (pWidget)
	{
		pWidget->Deselect();
	}
	pWidget = m_uUIWidgets->NextSelectableHorizontal(false);
	if (pWidget)
	{
		pWidget->Select();
	}
}

void UIContext::OnRight()
{
	if (m_bInteracting || m_uUIWidgets->NumColumns() < 2)
	{
		return;
	}
	UIWidget* pWidget = GetSelected();
	if (pWidget)
	{
		pWidget->Deselect();
	}
	pWidget = m_uUIWidgets->NextSelectableHorizontal(true);
	if (pWidget)
	{
		pWidget->Select();
	}
}

void UIContext::OnEnterPressed()
{
	UIWidget* pSelected = GetSelected();
	if (pSelected && pSelected->IsInteractable())
	{
		pSelected->InteractStart();
		m_bInteracting = true;
	}
}

void UIContext::OnEnterReleased(bool bInteract)
{
	if (m_bInteracting)
	{
		UIWidget* pSelected = GetSelected();
		if (pSelected)
		{
			pSelected->InteractEnd(bInteract);
		}
		m_bInteracting = false;
	}
}

void UIContext::OnBackReleased()
{
	if (m_bInteracting)
	{
		OnEnterReleased(false);
	}
	else
	{
		m_onCancelledDelegate();
		m_bDestroyed = m_bAutoDestroyOnCancel;
	}
}
} // namespace LittleEngine
