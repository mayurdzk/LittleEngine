#include "stdafx.h"
#include "LittleEngine/Debug/Tweakable.h"
#include "LEGame/GameFramework.h"
#include "TestWorld.h"

#include "SFMLAPI/System/SFTypes.h"
#include "SFMLAPI/Rendering/Primitives/Quad.h"
#include "SFMLAPI/Rendering/Primitives/Quads.h"
#include "SFMLAPI/Rendering/Primitives/SFText.h"
#include "SFMLAPI/Rendering/Primitives/SFRect.h"
#include "SFMLAPI/Rendering/Primitives/SFCircle.h"

namespace LittleEngine
{
extern bool g_bTerminateOnReady;

namespace
{
bool bSpawnColliderMinefield = false;
LEInput::Token t0;
TestWorld* pTestWorld = nullptr;

Entity *pEntity0 = nullptr, *pEntity1 = nullptr;
Entity *pEntity2 = nullptr, *pEntity3 = nullptr;
Entity* pEntity4 = nullptr;
Quads* pQuads0 = nullptr;
Quad* pQuad0 = nullptr;

// bool bLoopingPS = false;
ParticleSystem* pParticleSystem0 = nullptr;
ParticleSystem* pParticleSystem1 = nullptr;
Emitter::OnTick::Token psToken;

Deferred<TextureAsset*> largeTex;
Deferred<TextAsset*> miscText;
TextureAsset* pLargeTex = nullptr;
TextAsset* pMiscText = nullptr;
bool bStartedAsycLoad = false;
bool bLoadedLargeTex = false;
bool bLoadedMiscText = false;

Player* pPlayer = nullptr;
bool bParented = false;

void OnX()
{
	if (pEntity0 && pPlayer)
	{
		if (!bParented)
		{
			pEntity0->m_transform.SetParent(pPlayer->m_transform);
			bParented = true;
		}
		else
		{
			pEntity0->m_transform.UnsetParent();
			bParented = false;
		}
	}
}
void OnY()
{
	if (pEntity0 && pPlayer)
	{
		if (!bParented)
		{
			pEntity0->m_transform.SetParent(pPlayer->m_transform);
			bParented = true;
		}
		else
		{
			pEntity0->m_transform.UnsetParent();
			bParented = false;
		}
	}
}

void OnA()
{
	// Collision Test
	if (!pEntity2 && !pEntity3)
	{
		pEntity2 = g_pGameManager->NewEntity<Entity>("Yellow Circle", Vector2(-300, 300));
		if (pEntity2)
		{
			auto rc0 = pEntity2->AddComponent<RenderComponent>();
			rc0->SetCircle(LAYER_DEFAULT)->SetDiameter(200)->SetPrimaryColour(Colour::Yellow);
			auto t0 = pEntity2->AddComponent<CollisionComponent>();
			t0->AddCircle(200);
		}

		pEntity3 = g_pGameManager->NewEntity<Entity>("Blue Rectangle", Vector2(500, -200));
		if (pEntity3)
		{
			auto rc1 = pEntity3->AddComponent<RenderComponent>();
			rc1->SetRectangle(LAYER_DEFAULT)->SetSize({600, 100})->SetPrimaryColour(Colour::Blue);
			auto t1 = pEntity3->AddComponent<CollisionComponent>();
			t1->AddAABB(AABBData({600, 100}));
		}
	}
	else
	{
		pEntity2->Destruct();
		pEntity2 = nullptr;
		pEntity3->Destruct();
		pEntity3 = nullptr;
	}

	// SpriteSheet flipbook test
	if (!pEntity4)
	{
		pEntity4 = g_pGameManager->NewEntity<Entity>("SpriteSheetTest");
		pEntity4->m_transform.SetPosition({-200, -200});
		auto rc4 = pEntity4->AddComponent<RenderComponent>();
		rc4->SetSpriteSheet(SpriteSheet("Textures/TestSheet_64x64_6x6", Time::Seconds(1.0f)), LAYER_FX);
	}
	else
	{
		pEntity4->Destruct();
		pEntity4 = nullptr;
	}
}

void OnB()
{
	// Tiles
	static bool bShowTiles = false;
	bShowTiles = !bShowTiles;
	if (bShowTiles)
	{
		auto pTexture = g_pRepository->Load<TextureAsset>("Textures/Tiles/SpaceTile0.png");
		g_pGameManager->WorldCamera()->FillViewWithTiles(*pTexture);
	}
	else
	{
		g_pGameManager->WorldCamera()->ClearTiles();
	}
}

void OnEnter()
{
	// Fire0 Test
	if (!pParticleSystem1)
	{
		return;
	}

	if (pParticleSystem1->IsPlaying())
	{
		pParticleSystem1->Stop();
	}

	else
	{
		Fixed k(9, 10);
		Fixed x = Maths::Random::Range(-k, k);
		Fixed y = Maths::Random::Range(-k, k);
		Vector2 worldPos = g_pGameManager->Renderer()->Project({x, y}, false);
		pParticleSystem1->m_transform.SetPosition(worldPos);
		pParticleSystem1->Start();
		g_pGameManager->WorldCamera()->Shake();
	}
}

void OnSelect()
{
	// Stars0 Test
	if (!pParticleSystem0)
	{
		return;
	}

	if (pParticleSystem0->IsPlaying())
	{
		pParticleSystem0->Stop();
	}
	else
	{
		// pParticleSystem0->SetEnabled(true);
		// Fixed x = Maths::Random::Range(-Fixed::One, Fixed::One);
		// Fixed y = Maths::Random::Range(-Fixed::One, Fixed::One);
		// Vector2 worldPos = g_pGameManager->Renderer()->Project({x, y}, false);
		// pParticleSystem0->m_transform.localPosition = worldPos;
		pParticleSystem0->Start();
		// g_pGameManager->WorldCamera()->Shake();
	}
}

bool Test_OnInput(const LEInput::Frame& frame)
{
	if (frame.IsReleased({KeyType::JOY_BTN_0, KeyCode::Space}))
	{
		OnA();
	}

	if (frame.IsReleased({KeyType::JOY_BTN_1, KeyCode::E}))
	{
		OnB();
	}

	if (frame.IsReleased({KeyType::JOY_BTN_2, KeyCode::R}))
	{
		OnX();
	}

	if (frame.IsReleased({KeyType::JOY_BTN_3, KeyCode::F}))
	{
		OnY();
	}

	if (frame.IsReleased({KeyType::JOY_BTN_7, KeyCode::Enter}))
	{
		OnEnter();
	}

	if (frame.IsReleased({KeyType::JOY_BTN_4, KeyCode::Tab}))
	{
		OnSelect();
	}

	if (frame.IsHeld(KeyCode::Left) && frame.IsHeld({KeyCode::LShift, KeyCode::RShift}))
	{
		Transform& t = g_pGameManager->WorldCamera()->m_transform;
		t.SetPosition(t.GetWorldPosition() - Vector2(Fixed::Three, Fixed::Zero));
		return true;
	}

	if (frame.IsHeld(KeyCode::Right) && frame.IsHeld({KeyCode::LShift, KeyCode::RShift}))
	{
		Transform& t = g_pGameManager->WorldCamera()->m_transform;
		t.SetPosition(t.GetWorldPosition() + Vector2(Fixed::Three, Fixed::Zero));
		return true;
	}
	return false;
}

void SpawnColliderMinefield()
{
	u32 id = 0;
	for (Fixed x = -600; x < 800; x += 200)
	{
		for (Fixed y = -350; y < 500; y += 200)
		{
			String name = "ColliderMine_" + Strings::ToString(id++);
			auto* pE = g_pGameManager->NewEntity<Entity>(name, Vector2(x, y));
			auto* pCC = pE->AddComponent<CollisionComponent>();
			pCC->AddAABB(AABBData({100, 100}));
		}
	}
}

void StartTests()
{
	pEntity0 = g_pGameManager->NewEntity<Entity>("Entity0", {300, 200});
	auto rc0 = pEntity0->AddComponent<RenderComponent>();
	rc0->SetRectangle(LAYER_DEFAULT)
		->SetSize({300, 100})
		->SetPrimaryColour(Colour::Cyan)
		->SetEnabled(true);

	pEntity1 = g_pGameManager->NewEntity<Entity>(
		"Entity1", g_pGameManager->Renderer()->Project({0, Fixed(0.9f)}, false));
	auto rc1 = pEntity1->AddComponent<RenderComponent>();
	FontAsset* font = g_pRepository->GetDefaultFont();
	rc1->SetText(LAYER_DEFAULT)
		->SetText("Hello World!")
		->SetFont(*font)
		->SetSize(50)
		->SetPrimaryColour(Colour(200, 150, 50))
		->SetEnabled(true);

	pPlayer = g_pGameManager->NewEntity<Player>("Player0", {0, -300});
	auto* pTexture = g_pRepository->Load<TextureAsset>("Textures/Ship.png");
	PlayerData data(
		*pTexture, {PlayerCollider(AABBData({120, 60}), {0, -15}), PlayerCollider(AABBData({60, 80}))});
	pPlayer->InitPlayer(std::move(data));

	String psName = "Stars0";
	String path = "VFX/Stars0/Stars0.psdata";
	auto* pText = g_pRepository->Load<TextAsset>(path);
	GData psGData(pText->GetText());
	pParticleSystem0 = g_pGameManager->NewEntity<ParticleSystem>(std::move(psName));
	pParticleSystem0->InitParticleSystem(ParticleSystemData(psGData));
	pParticleSystem0->Stop();
	Emitter* pEmitter0 = pParticleSystem0->GetEmitter("main");
	if (pEmitter0)
	{
		psToken = pEmitter0->RegisterOnTick([](Particle& p) {
			static Vector2 target = g_pGameManager->Renderer()->Project({Fixed::OneHalf, -Fixed(1.5f)}, false);
			Vector2 wind = (target - p.m_transform.GetWorldPosition()).Normalised();
			p.m_v.x = wind.x * Maths::Abs(p.m_v.y) * Fixed::OneHalf;
		});
	}

	pText = g_pRepository->Load<TextAsset>("VFX/Fire0/Fire0_noloop.psdata");
	pParticleSystem1 = g_pGameManager->NewEntity<ParticleSystem>("Fire0");
	pParticleSystem1->InitParticleSystem(ParticleSystemData(GData(pText->GetText())));
	pParticleSystem1->m_transform.SetScale({Fixed::Two, Fixed::Two});
	pParticleSystem1->Stop();

	if (bSpawnColliderMinefield)
	{
		SpawnColliderMinefield();
	}

	pPlayer->GetComponent<RenderComponent>()->SetShader<Shader>("Default");
	
	auto pTex = g_pRepository->Load<TextureAsset>("Misc/Test.png"); 
	if (pTex)
	{
		pQuads0 = g_pGameManager->Renderer()->New<Quads>(LAYER_DEFAULT);
		pTex->SetRepeated(true);
		pQuads0->SetTexture(*pTex)->SetEnabled(true);
		pQuad0 = pQuads0->AddQuad();
		pQuad0->SetPosition({600, 300}, true)->SetEnabled(true)->SetScale({Fixed::OneHalf, Fixed::One}, true);
	}
}

UIButtonDrawer* pButtonDrawer = nullptr;
bool bModal = true;
bool bSpawnedDrawer = false;
Vec<LEInput::Token> debugTokens;

UIDialogue* pDialogue = nullptr;

// Fixed progress;
UPtr<UIProgressBar> uProgressBar = nullptr;
UPtr<UIElement> uProgressBG = nullptr;
Time elapsed = Time::Zero;

void SpawnDialogue()
{
	pDialogue = g_pGameManager->UI()->PushContext<UIDialogue>("TestUIDialogue");
	pDialogue->SetHeader(UIText("Title", 25, Colour::Black));
	pDialogue->SetContent(UIText("Content goes here", 15, Colour::Black), &Colour::White);
	debugTokens.push_back(pDialogue->AddMainButton("OK", []() { LOG_D("OK pressed!"); }, false));
	debugTokens.push_back(pDialogue->AddOtherButton("Cancel", []() { pDialogue->Destruct(); }, false));
	pDialogue->SetActive(true);
}

void SpawnToggle()
{
	Fixed x = 400;
	Fixed y = 200;
	auto* pParent = g_pGameManager->UI()->PushContext<UIContext>("TestToggleUIC");
	pParent->GetRoot()->m_transform.size = {x, y};
	UIWidgetStyle toggleStyle = UIGameStyle::GetStyle("");
	toggleStyle.widgetSize = {x, y * Fixed::OneHalf};
	toggleStyle.background = Colour::Yellow;
	auto* pToggle0 = pParent->AddWidget<UIToggle>("Toggle0", &toggleStyle);
	auto* pToggle1 = pParent->AddWidget<UIToggle>("Toggle1", &toggleStyle);

	debugTokens.push_back(pToggle0->AddCallback([](bool bVal) { LOG_W("Toggle0 changed! %d", bVal); }));
	pToggle0->GetRoot()->m_transform.bAutoPad = true;
	pToggle0->SetBackground(Colour::White)
		->SetText("Toggle 0")
		->SetOn(false)
		->GetRoot()
		->m_transform.nPosition = {0, 1};
	debugTokens.push_back(
		pToggle1->AddCallback([](bool bValue) { LOG_W("Toggle1 changed! %d", bValue); }));
	pToggle1->GetRoot()->m_transform.bAutoPad = true;
	pToggle1->SetBackground(Colour::White)->SetText("Toggle 1")->SetOn(true)->GetRoot()->m_transform.nPosition = {
		0, -1};

	pParent->m_bAutoDestroyOnCancel = true;
	pParent->SetActive(true);
}

void LoadAsyncTest()
{
	pLargeTex = nullptr;
	pMiscText = nullptr;
	largeTex = g_pRepository->LoadAsync<TextureAsset>("Misc/06.png");
	miscText = g_pRepository->LoadAsync<TextAsset>("Misc/BinaryText.txt");
	bStartedAsycLoad = true;
	bLoadedLargeTex = bLoadedMiscText = false;
}

void TestTick(Time dt)
{
	elapsed += dt;

	if (elapsed.AsSeconds() >= 0.25f && g_bTerminateOnReady)
	{
		g_pGameManager->Quit();
		return;
	}

	if (elapsed.AsSeconds() >= 1 && !bSpawnedDrawer)
	{
		bSpawnedDrawer = true;
		pButtonDrawer = g_pGameManager->UI()->PushContext<UIButtonDrawer>("TestUIButtonDrawer");
		pButtonDrawer->m_bAutoDestroyOnCancel = !bModal;
		UIStyle panelStyle;
		panelStyle.size = {500, 600};
		panelStyle.fill = Colour(100, 100, 100, 100);
		pButtonDrawer->SetPanel(panelStyle);
		UIButton* pButton1 = nullptr;
		debugTokens.push_back(pButtonDrawer->AddButton("Load Async", &LoadAsyncTest));
		debugTokens.push_back(pButtonDrawer->AddButton(
			"Options", []() { g_pGameManager->UI()->PushContext<UIOptions>("Options"); }, &pButton1));
		pButton1->SetInteractable(false);
		debugTokens.push_back(pButtonDrawer->AddButton("Toggle B1", [pButton1]() {
			pButton1->SetInteractable(!pButton1->IsInteractable());
			LOG_D("Button 1 Set Interactable: %s", Strings::ToString(pButton1->IsInteractable()).c_str());
		}));
		/*debugTokens.push_back(
			pButtonDrawer->AddButton("Button 3", []() { LOG_D("Button 3 pressed!"); }));*/
		debugTokens.push_back(pButtonDrawer->AddButton("Toggle", &SpawnToggle));
		debugTokens.push_back(pButtonDrawer->AddButton("Dialogue", []() { SpawnDialogue(); }));
		if (bModal)
		{
			debugTokens.push_back(
				pButtonDrawer->AddButton("Cancel", []() { pButtonDrawer->Destruct(); }));
		}
		pButtonDrawer->SetActive(true);
	}

	static bool bSpawnedTextInput = false;
	static UIContext* pTextContext = nullptr;
	static UITextInput* pTextInput = nullptr;
	if (elapsed.AsSeconds() >= 2 && !bSpawnedTextInput)
	{
		pTextContext = g_pGameManager->UI()->PushContext<UIContext>("TestTextInputUIC");
		pTextInput = pTextContext->AddWidget<UITextInput>("TextInput");
		pTextContext->m_bAutoDestroyOnCancel = true;
		pTextContext->SetActive(true);
		bSpawnedTextInput = true;
	}

	static u32 frame = 0;
	if (bStartedAsycLoad && (!bLoadedLargeTex || !bLoadedMiscText))
	{
		bLoadedLargeTex |= largeTex.IsReady();
		bLoadedMiscText |= miscText.IsReady();
		if (!bLoadedLargeTex)
		{
			LOG_I("Waiting to load %s, frame #%d", "Misc/06.png", frame);
		}
		else if (!pLargeTex)
		{
			pLargeTex = largeTex.Get();
			if (pLargeTex)
			{
				LOG_I("Loaded %s", pLargeTex->GetID());
			}
		}
		if (!bLoadedMiscText)
		{
			LOG_I("Waiting to load %s, frame %d", "Misc/BinaryText.txt", frame);
		}
		else if (!pMiscText)
		{
			pMiscText = miscText.Get();
			if (pMiscText)
			{
				LOG_I("Loaded %s: %s", "Misc/BinaryText.txt", pMiscText->GetText().c_str());
			}
		}
		++frame;
	}

	if (pQuad0)
	{
		static f32 u;
		u += dt.AsSeconds() * 0.1f;
		pQuad0->SetUV(Fixed(u), Fixed::Zero, Fixed(u) + Fixed::OneHalf, Fixed::One);
		Fixed o = Vector2::ToOrientation(pQuad0->GetOrientation());
		Vector2 _o = Vector2::ToOrientation(o + 2);
		pQuad0->SetOrientation(_o);
	}

	/*PROFILE_CUSTOM("TEST", Time::Milliseconds(3), Colour::White);
	std::this_thread::sleep_for(std::chrono::milliseconds(Maths::Random::Range(0, 3)));
	PROFILE_STOP("TEST");*/
}

void Cleanup()
{
	elapsed = Time::Zero;
	pTestWorld = nullptr;

	pParticleSystem0 = nullptr;
	pParticleSystem1 = nullptr;
	pPlayer = nullptr;

	pEntity0 = pEntity1 = pEntity2 = pEntity3 = pEntity4 = nullptr;

	bSpawnedDrawer = false;

	if (uProgressBar)
	{
		uProgressBar = nullptr;
	}
	if (uProgressBG)
	{
		uProgressBG = nullptr;
	}
	psToken = nullptr;
	if (pQuads0)
	{
		pQuads0->Destroy();
	}
	pQuads0 = nullptr;
	pQuad0 = nullptr;
	debugTokens.clear();
}
} // namespace

TestWorld::TestWorld() : World("Test")
{
}

void TestWorld::OnActivated()
{
	pTestWorld = this;
	StartTests();
	BindInput([&](const LEInput::Frame& frame) -> bool { return OnInput(frame); });
	BindInput(&Test_OnInput);
}

void TestWorld::PreTick(Time dt)
{
	TestTick(dt);
}

void TestWorld::PostTick(Time /*dt*/)
{
}

void TestWorld::OnDeactivating()
{
	Cleanup();
}

bool TestWorld::OnInput(const LEInput::Frame& frame)
{
	if (frame.IsReleased({KeyType::JOY_BTN_6, KeyCode::Escape}))
	{
		g_pGameManager->UI()->PushContext<UIOptions>("Options");
		return true;
	}
	return false;
}

} // namespace LittleEngine
