#include "stdafx.h"
#include "Core/ArchiveReader.h"
#include "Core/GData.h"
#include "Core/Logger.h"
#include "SFMLAPI/System/SFAssets.h"
#include "LittleEngine/Debug/Console/DebugCommands.h"
#include "LittleEngine/Debug/Console/LogLine.h"
#include "LittleEngine/Game/GameManager.h"
#include "LittleEngine/Repository/EngineRepository.h"
#include "LittleEngine/Services/Services.h"
#include "LittleEngine/UI/UIElement.h"
#include "LittleEngine/UI/UIManager.h"
#include "LittleEngine/UI/UIText.h"
#include "LittleEngine/UI/UIWidget.h"
#include "LittleEngine/UI/UIWidgetMatrix.h"
#include "UIContainer.h"
#include "GameFramework/UI/Widgets/UIButton.h"
#include "GameFramework/UI/Widgets/UISelection.h"
#include "GameFramework/UI/Widgets/UITextInput.h"
#include "GameFramework/UI/Widgets/UIToggle.h"

namespace LittleEngine
{
using GData = Core::GData;

namespace
{
UByte H2B(const String& hex)
{
	s32 integer = -1;
	std::istringstream(hex) >> std::hex >> integer;
	if (integer >= 0)
	{
		return UByte(static_cast<u8>(integer));
	}
	return UByte(255);
}

Colour HexStrToColour(String hex, Colour defaultColour = Colour::White)
{
	Strings::ToLower(hex);
	if (hex.size() == 6 || hex.size() == 8)
	{
		String r = hex.substr(0, 2);
		String g = hex.substr(2, 2);
		String b = hex.substr(4, 2);
		String a = "ff";
		if (hex.size() == 8)
		{
			a = hex.substr(6, 2);
		}
		return Colour(H2B(r), H2B(g), H2B(b), H2B(a));
	}
	return defaultColour;
}
} // namespace

#if ENABLED(CONSOLE)
namespace Debug
{
extern Colour g_logTextColour;
extern Colour g_logWarningColour;

namespace Commands
{
CMD_UIContainer::CMD_UIContainer() : Command("ui_container")
{
	m_bTakesCustomParam = true;
}

void CMD_UIContainer::FillExecuteResult(String params)
{
	if (params.empty())
	{
		m_executeResult.emplace_back(
			"Syntax: " + m_name + " <assetID> / clear / reset (if UIContainer no longer exists)", g_logTextColour);
	}
	else
	{
		if (params == "clear")
		{
			if (m_pContainer)
			{
				m_pContainer->Destruct();
				m_pContainer = nullptr;
			}
		}
		else if (params == "reset")
		{
			m_pContainer = nullptr;
		}
		else
		{
			TextAsset* pGData = Services::Game()->Repository()->Load<TextAsset>(params, true);
			if (!pGData)
			{
				m_executeResult.emplace_back("Asset ID not found: " + params, g_logWarningColour);
			}
			else
			{
				m_pContainer =
					Services::Game()->UI()->PushContext<UIContainer>("ConsoleUIContainer");
				if (m_pContainer)
				{
					m_pContainer->Deserialise(pGData->GetText());
					m_pContainer->SetActive(true);
				}
			}
		}
	}
};
} // namespace Commands
} // namespace Debug
#endif

Colour UIContainer::ParseColour(String serialised)
{
	Strings::ToLower(serialised);
	if (serialised == "white")
	{
		return Colour::White;
	}
	if (serialised == "black")
	{
		return Colour::Black;
	}
	if (serialised == "red")
	{
		return Colour::Red;
	}
	if (serialised == "green")
	{
		return Colour::Green;
	}
	if (serialised == "blue")
	{
		return Colour::Blue;
	}
	if (serialised == "cyan")
	{
		return Colour::Cyan;
	}
	if (serialised == "magenta")
	{
		return Colour::Magenta;
	}
	if (serialised == "yellow")
	{
		return Colour::Yellow;
	}
	if (serialised == "transparent")
	{
		return Colour::Transparent;
	}
	if (!serialised.empty() && serialised[0] == '#')
	{
		return HexStrToColour(serialised.substr(1, serialised.length() - 1));
	}
	return Colour::White;
}

UIContainer::UIContainer() = default;
UIContainer::~UIContainer() = default;

void UIContainer::Deserialise(String serialised)
{
	GData root(std::move(serialised));
	m_bAutoDestroyOnCancel = root.GetBool("isDestroyOnCancel", true);
	// Setup root
	m_pRootElement->m_transform.size = root.GetVector2("size");
	if (root.GetBool("isPanel"))
	{
		m_pRootElement->SetPanel(ParseColour(root.GetString("colour")));
	}
	// Setup children
	SetupChildren(m_pRootElement, root.GetVectorGData("children"));
}

void UIContainer::SetupChildren(UIElement* pParent, Vec<GData> uiObjects)
{
	for (auto& data : uiObjects)
	{
		UIObject* pObj = nullptr;
		UIElement* pNextParent = nullptr;
		s32 layerDelta = data.GetS32("layerDelta", 0);
		String id = data.GetString("id");
		String name = String(GetNameStr()) + "_" + id;
#if !SHIPPING
		if (!id.empty())
		{
			auto duplicate = std::find_if(m_objs.begin(), m_objs.end(),
										  [&id](const Obj& obj) { return obj.id == id; });
			Assert(duplicate == m_objs.end(),
				   "Duplicate ID encountered while deserialising UIContainer!");
			if (duplicate != m_objs.end())
			{
				LOG_E("[UIContainer] Duplicate ID encountered! [%s]", id.c_str());
			}
		}
#endif
		String widgetType = data.GetString("widgetType");
		Vector2 position = data.GetVector2("position", Vector2::Zero);
		Vector2 size = data.GetVector2("size", {-1, -1});
		GData uiTextData = data.GetString("uiText");
		String text = uiTextData.GetString("text");
		u32 textSize = uiTextData.GetS32("size", UIText::s_DEFAULT_PIXEL_SIZE);
		Colour textColour = ParseColour(uiTextData.GetString("colour"));
		if (text.empty())
		{
			text = data.GetString("text");
		}

		Strings::ToLower(widgetType);
		if (widgetType.empty())
		{
			UIElement* pElement = AddElement<UIElement>(name, &pParent->m_transform, layerDelta);
			pElement->m_transform.bAutoPad = data.GetBool("isAutoPad");
			pElement->m_transform.nPosition = position;
			if (size.x > Fixed::Zero && size.y > Fixed::Zero)
			{
				pElement->m_transform.size = size;
			}
			if (!text.empty())
			{
				pElement->SetText(UIText(text, textSize, textColour));
			}
			if (data.GetBool("isPanel"))
			{
				pElement->SetPanel(ParseColour(data.GetString("colour")), data.GetS32("border", 0),
								   ParseColour(data.GetString("outline")));
			}

			pNextParent = pElement;
			pObj = pElement;
		}
		else
		{
			bool bNewColumn = data.GetBool("isNewColumn", false);
			UIWidgetStyle style0 = UIWidgetStyle::GetDefault0();
			UIWidgetStyle style1 = UIWidgetStyle::GetDefault1();
			UIWidget* pWidget = nullptr;
			UIWidgetStyle* pStyle = nullptr;

			switch (data.GetS32("widgetStyle", -1))
			{
			case 0:
				pStyle = &style0;
				break;

			case 1:
				pStyle = &style1;
				break;
			}

			if (widgetType == "button")
			{
				if (size.x > Fixed::Zero && size.y > Fixed::Zero)
				{
					if (!pStyle)
					{
						pStyle = &style0;
					}
					pStyle->widgetSize = size;
				}
				UIButton* pButton = AddWidget<UIButton>(name, pStyle, bNewColumn);
				if (!text.empty())
				{
					pButton->SetText(UIText(text, textSize, textColour));
				}
				pWidget = pButton;
			}

			else if (widgetType == "toggle")
			{
				UIToggle* pToggle = AddWidget<UIToggle>(name, pStyle, bNewColumn);
				pToggle->SetOn(data.GetBool("isOn", true));
				if (size.x > 0 & size.y > 0)
				{
					pToggle->SetBoxSize(size);
				}
				if (!text.empty())
				{
					pToggle->SetText(UIText(text, textSize, textColour));
				}
				String dump = data.GetString("onColour");
				if (!dump.empty())
				{
					pToggle->SetOnColour(ParseColour(dump));
				}
				dump = data.GetString("offColour");
				if (!dump.empty())
				{
					pToggle->SetOffColour(ParseColour(dump));
				}
				dump = data.GetString("background");
				if (!dump.empty())
				{
					pToggle->SetBackground(ParseColour(dump));
				}
				pWidget = pToggle;
			}

			else if (widgetType == "selection")
			{
				UISelection* pSelection = AddWidget<UISelection>(name, pStyle, bNewColumn);
				pSelection->SetText(UIText(text, textSize, textColour));
				pSelection->SetOptions(data.GetVector("options"));
				pWidget = pSelection;
			}

			else if (widgetType == "textInput")
			{
				UITextInput* pTextInput = AddWidget<UITextInput>(name, pStyle, bNewColumn);
				pWidget = pTextInput;
			}

			else
			{
				LOG_E("[UIContainer] Unsupported UIWidget type! [%s]", widgetType.c_str());
			}

			// Generic Widget setup
			if (pWidget)
			{
				pWidget->SetInteractable(data.GetBool("isInteractable", true));
				pObj = pWidget;

				pNextParent = pWidget->GetRoot();
				pNextParent->m_transform.nPosition = position;
				pNextParent->m_transform.bAutoPad = data.GetBool("isAutoPad", false);
				pNextParent->m_transform.padding = data.GetVector2("padding", Vector2::Zero);
				if (size.x > 0 && size.y > 0)
				{
					pNextParent->m_transform.size = size;
				}	
			}
		}

		pNextParent->SetParent(pParent->m_transform);

		if (pObj)
		{
			m_objs.push_back({id, pObj});
		}

		SetupChildren(pNextParent, data.GetVectorGData("children"));
	}
}

UIObject* UIContainer::GetObj(const String& id)
{
	auto search =
		std::find_if(m_objs.begin(), m_objs.end(), [&id](const Obj& obj) { return obj.id == id; });
	if (search != m_objs.end())
	{
		return search->pObj;
	}
	return nullptr;
}
} // namespace LittleEngine