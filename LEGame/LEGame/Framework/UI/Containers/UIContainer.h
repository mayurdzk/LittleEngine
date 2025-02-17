#pragma once
#include "SFMLAPI/Rendering/Colour.h"
#include "LEGame/Model/UI/UIContext.h"
#if ENABLED(CONSOLE)
#include "LEGame/Utility/Debug/Console/DebugCommands.h"
#endif

namespace Core
{
class GData;
}

namespace LittleEngine
{
class UIContainer : public UIContext
{
private:
	struct Obj
	{
		String id;
		UIObject* pObj;
	};

private:
	Vec<Obj> m_objs;

public:
	UIContainer();
	~UIContainer() override;

	void Deserialise(String serialised);

	UIObject* GetObj(const String& id);
	template <typename T>
	T* Find(const String& id);

private:
	void SetupChildren(class UIElement* pParent, Vec<Core::GData> uiObjects);
};

template <typename T>
T* UIContainer::Find(const String& id)
{
	static_assert(IsDerived<UIObject, T>, "T must derive from UIObject!");
	return dynamic_cast<T*>(GetObj(id));
}

#if ENABLED(CONSOLE)
namespace Debug::Commands
{
class CMD_UIContainer : public Command
{
private:
	UIContainer* m_pContainer = nullptr;

public:
	CMD_UIContainer();

	void FillExecuteResult(String params) override;
};
} // namespace Debug::Commands
#endif
} // namespace LittleEngine
