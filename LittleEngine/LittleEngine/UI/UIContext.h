#pragma once
#include "CoreTypes.h"
#include "Delegate.hpp"
#include "SFMLAPI/Rendering/SFLayerID.h"
#include "UIObject.h"
#include "LittleEngine/Input/EngineInput.h"

namespace LittleEngine
{
// \brief Controller for a number of UIWidgets: allows player to cycle through and interact with all of them
class UIContext : public UIObject
{
public:
	using OnCancelled = Core::Delegate<>;

private:
	using UUIWidget = UPtr<class UIWidget>;
	using UUIElement = UPtr<class UIElement>;

public:
	bool m_bAutoDestroyOnCancel = false;
protected:
	UIElement* m_pRootElement = nullptr;
private:
	UPtr<class UIWidgetMatrix> m_uUIWidgets;
	Vec<UUIElement> m_uiElements;
	Vec<EngineInput::Token> m_inputTokens;
	OnCancelled m_onCancelledDelegate;
	bool m_bInteracting = false;

public:
	UIContext();
	UIContext(const String& name);
	virtual ~UIContext();

	template <typename T>
	T* AddWidget(const String& name, struct UIWidgetStyle* pStyleToCopy = nullptr, bool bNewColumn = false);
	template <typename T>
	T* AddElement(const String& name, struct UITransform* pParent = nullptr);

	void SetActive(bool bActive, bool bResetSelection = true);
	void ResetSelection();
	UIWidget* GetSelected();
	UIElement* GetRootElement() const;
	OnCancelled::Token SetOnCancelled(const OnCancelled::Callback& Callback, bool bAutoDestroy);
	void Destruct();

	virtual void Tick(Time dt) override;

protected:
	virtual void OnInitContext();
	virtual void OnDestroying();

	bool OnInput(const EngineInput::Frame& frame);

	void OnUp();
	void OnDown();
	void OnLeft();
	void OnRight();
	void OnEnterPressed();
	void OnEnterReleased(bool bInteract);
	void OnBackReleased();
	void Discard();

private:
	void InitContext(LayerID rootLayer);
	LayerID GetMaxLayer() const;

	friend class UIManager;
};

template <typename T>
T* UIContext::AddWidget(const String& name, UIWidgetStyle* pStyleToCopy, bool bNewColumn)
{
	static_assert(std::is_base_of<UIWidget, T>::value, "T must derive from UIWidget.");
	UPtr<T> uT = MakeUnique<T>(name);
	T* pT = uT.get();
	UIWidgetStyle defaultStyle = UIWidgetStyle::GetDefault0();
	if (!pStyleToCopy)
		pStyleToCopy = &defaultStyle;
	pStyleToCopy->baseLayer = static_cast<LayerID>(m_pRootElement->m_layer + 1);
	uT->InitWidget(*this, pStyleToCopy);
	m_uUIWidgets->EmplaceWidget(std::move(uT), bNewColumn);
	LOG_D("%s %s", pT->LogNameStr(), "constructed");
	return pT;
}

template <typename T>
T* UIContext::AddElement(const String& name, UITransform* pParent)
{
	static_assert(std::is_base_of<UIElement, T>::value, "T must derive from UIWidget!");
	UPtr<T> uT = MakeUnique<T>(name, true);
	if (!pParent && m_pRootElement)
		pParent = &m_pRootElement->m_transform;
	uT->InitElement(pParent);
	if (m_pRootElement)
		uT->m_layer = static_cast<LayerID>(m_pRootElement->m_layer + 1);
	T* pT = uT.get();
	m_uiElements.push_back(std::move(uT));
	LOG_D("%s %s", pT->LogNameStr(), "constructed");
	return pT;
}
} // namespace LittleEngine
