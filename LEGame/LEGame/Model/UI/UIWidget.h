#pragma once
#include "Core/CoreTypes.h"
#include "LEGame/Model/UI/UIObject.h"
#include "LEGame/Model/UI/UIStyle.h"

namespace LittleEngine
{
enum class UIWidgetState
{
	Uninteractable,
	NotSelected,
	Selected,
	Interacting
};

// \brief Base UI interactable class: must be used in conjunction with an owning UIContext
class UIWidget : public UIObject
{
protected:
	UIWidgetStyle m_style;
	Vec<UPtr<class UIElement>> m_uiElements;
	class UIContext* m_pOwner = nullptr;
	class UIElement* m_pRoot = nullptr;
	UIWidgetState m_state = UIWidgetState::NotSelected;
	UIWidgetState m_prevState;

public:
	UIWidget();
	~UIWidget() override;

	template <typename T>
	UIElement* AddElement(String name = "", struct UITransform* pParent = nullptr, s32 layerDelta = 0);

	UIWidgetStyle& GetStyle();
	UIElement* GetRoot() const;

public:
	virtual void SetStyle(const UIWidgetStyle& style);
	void SetInteractable(bool bInteractable);
	bool IsInteractable() const;

protected:
	void SetState(UIWidgetState state);

public:
	void Tick(Time dt = Time::Zero) override;

protected:
	virtual bool IsPointInBounds(Vector2 point) const;
	virtual void OnCreated();
	virtual void OnSelected() = 0;
	virtual void OnDeselected() = 0;
	virtual void OnInteractStart() = 0;
	virtual void OnInteractEnd(bool bInteract) = 0;
	virtual void OnSetInteractable(bool bInteractable) = 0;

// UIContext friend functions
private:
	void Select();
	void Deselect();
	void InteractStart();
	void InteractEnd(bool bInteract);

private:
	void OnCreate(String name, UIContext& owner, UIWidgetStyle* pStyleToCopy);
	void InitElement(String name, UIElement* pNewElement, UITransform* pParent);
	LayerID GetMaxLayer() const;

	friend class UIContext;
};

template <typename T>
UIElement* UIWidget::AddElement(String name, UITransform* pParent, s32 layerDelta)
{
	static_assert(std::is_base_of<UIElement, T>::value,
				  "T must derive from UIElement. Check Output Window for erroneous call");
	UPtr<T> uT = MakeUnique<T>(static_cast<LayerID>(m_style.baseLayer + layerDelta), false);
	T* pT = uT.get();
	m_uiElements.push_back(std::move(uT));
	InitElement(std::move(name), pT, pParent);
	return pT;
}
} // namespace LittleEngine
