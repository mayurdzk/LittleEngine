#pragma once
#include "UIContext.h"

namespace LittleEngine
{
class UIManager final
{
public:
private:
	Vector<UPtr<UIContext>> m_uContexts;

public:
	UIManager();
	~UIManager();

	template <typename T>
	T* SpawnContext(LayerID baseLayer = LAYER_UI)
	{
		static_assert(IsDerived(UIContext, T), "T must derive from UIContext!");
		if (!m_uContexts.empty())
		{
			auto& uHead = m_uContexts.back();
			uHead->SetActive(false);
		}
		UPtr<T> uT = MakeUnique<T>();
		uT->InitContext(baseLayer);
		uT->SetActive(true);
		T* pT = dynamic_cast<T*>(uT.get());
		m_uContexts.emplace_back(std::move(uT));
		LOG_D("%s %s", pT->LogNameStr(), "constructed");
		return pT;
	}

	UIContext* GetActiveContext() const;

	void Tick(Time dt);
};
} // namespace LittleEngine
