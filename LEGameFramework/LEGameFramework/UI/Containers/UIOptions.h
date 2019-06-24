#pragma once
#include "LEGameFramework/UI/Widgets/UIButton.h"
#include "UIContainer.h"

namespace LittleEngine
{
class UIOptions final : public UIContainer
{
private:
	Vec<UIButton::OnClick::Token> m_tokens;

public:
	UIOptions();
	~UIOptions() override;

protected:
	void OnCreated() override;
};
} // namespace LittleEngine
