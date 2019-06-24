#include "stdafx.h"
#include "Core/Logger.h"
#include "LittleEngine/FatalEngineException.h"
#include "LittleEngine/Debug/Profiler.h"
#include "LEGameModel/GameConfig.h"
#include "LEGameModel/World/WorldClock.h"
#include "LEGameFramework/Framework.h"
#include "LEGameFramework/Utility/Debug/Console/DebugConsole.h"
#include "LEGameFramework/Utility/Debug/DebugProfiler.h"
#include "LEGameCode/GameInit.h"
#include "GameLoop.h"

namespace LittleEngine
{
using namespace Debug;

namespace
{
using namespace Core;
using namespace LittleEngine;

// Globals
UPtr<LERepository> uRepository;
UPtr<LEShaders> uShaders;
UPtr<LEAudio> uAudio;

// LEContext
GameConfig config;
UPtr<LEContext> uContext;
UPtr<WorldStateMachine> uWSM;

// Game Loop
Time tickRate;
Time maxFrameTime;
bool bPauseOnFocusLoss = false;
bool bRenderThread = true;

TweakS32(ticksPerSec, nullptr);

bool Init(s32 argc, char** argv)
{
	OS::Env()->SetVars(argc, argv);
#if ENABLED(TWEAKABLES)
	ticksPerSec.BindCallback([](const String& val) {
		s32 newRate = Strings::ToS32(val);
		if (newRate > 0 && newRate < 250)
		{
			tickRate = Time::Seconds(1.0f / newRate);
			uContext->ModifyTickRate(tickRate);
			LOG_I("[GameLoop] Tick Rate changed to %.2f ms", tickRate.AsSeconds() * 1000.0f);
		}
		else
		{
			LOG_W("[GameLoop] Invalid value for ticks per second: %s", val.c_str());
		}
	});
#endif
	try
	{
		uRepository = MakeUnique<LERepository>("Fonts/main.ttf", "GameAssets.cooked", "GameAssets");
		uAudio = MakeUnique<LEAudio>();
	}
	catch (const FatalEngineException& /*e*/)
	{
		LOG_E("[GameLoop] ERROR! Could not initialise Engine Service!");
		return false;
	}

	config.Init();
#if !SHIPPING
	LOG_D("[GameLoop] Initialising event loop, loading config...");
	config.Load(".game.conf");
#endif
	auto pSettings = GameSettings::Instance();
	
	Core::g_MinLogSeverity = pSettings->GetLogLevel();
	bPauseOnFocusLoss = config.ShouldPauseOnFocusLoss();
	bRenderThread = config.ShouldCreateRenderThread();
	ControllerComponent::s_orientationEpsilon = config.GetControllerOrientationEpsilon();
#if DEBUGGING
	ControllerComponent::s_orientationWidthHeight = config.GetControllerOrientationSize();
	Entity::s_orientationWidthHeight = config.GetEntityOrientationSize();
#endif

	if (OS::Threads::GetVacantThreadCount() > 0)
	{
		String header = "Game: " + GameConfig::GetGameVersion().ToString() +
						" Engine : " + GameConfig::GetEngineVersion().ToString();
		u8 backupCount = config.GetBackupLogFileCount();
		Core::StartFileLogging("Debug", backupCount, std::move(header));
	}

	if (bRenderThread)
	{
		if (OS::Threads::GetVacantThreadCount() == 0)
		{
			LOG_W(
				"[GameLoop] Insufficient threads to create render thread!\n!ERROR! Async Renderer "
				"not "
				"available!");
			bRenderThread = false;
		}
	}

	Core::Jobs::Init(config.GetJobWorkerCount());

#if DEBUGGING
	Collider::s_debugShapeWidth = config.GetColliderBorderWidth();
#endif
	tickRate = config.GetTickRate();
	maxFrameTime = config.GetMaxFrameTime();

	uShaders = MakeUnique<LEShaders>();

	Time::Reset();

	return true;
}

void CreateContext(GameConfig& config)
{
	GameSettings& settings = *GameSettings::Instance();
	LEContextData data;
	Vector2 viewSize = config.GetViewSize();
	data.viewportData.viewportSize = settings.GetViewportSize(viewSize);
	data.viewportData.viewSize = viewSize;
	data.viewportData.title = config.GetTitleBarText();
	data.viewportData.style = settings.GetViewportStyle();
	data.tickRate = tickRate;
	data.bRenderThread = bRenderThread;
	data.renderThreadStartDelay = config.GetRenderThreadStartDelay();
	data.bPauseOnFocusLoss = config.ShouldPauseOnFocusLoss();
	Core::Property::Persistor inputMapPersistor;
	if (inputMapPersistor.Load(settings.GetValue("CUSTOM_INPUT_MAP")))
	{
		u16 count = data.inputMap.Import(inputMapPersistor);
		if (count > 0)
		{
			LOG_I("[GameLoop] Loaded %u custom Input Mappings successfully", count);
		}
	}
	uContext = MakeUnique<LEContext>(std::move(data));
}

bool Tick(Time dt)
{
	PROFILE_START("TICK", Colour(127, 0, 255));
	uRepository->Tick(dt);
	bool bYield = uWSM->Tick(dt);
	bYield |= uContext->Update();
	uAudio->Tick(dt);
#if ENABLED(CONSOLE)
	Console::Tick(dt);
#endif
	PROFILE_STOP("TICK");
#if ENABLED(PROFILER)
	Profiler::Tick(dt);
#endif

	return bYield;
}

#if DEBUGGING
inline void ProfileFrameTime(Time frameElapsed, Time maxFrameTime)
{
	static const Time DILATED_TIME = Time::Milliseconds(250);
	static const u8 MAX_CONSECUTIVE = 2;
	static Time logTime = Time::Now() - Time::Milliseconds(300);
	static u8 consecutive = 0;
	if (frameElapsed > maxFrameTime)
	{
		++consecutive;
		if ((Time::Now() - logTime) > DILATED_TIME && consecutive > MAX_CONSECUTIVE)
		{
			f32 max = maxFrameTime.AsSeconds() * 1000.0f;
			f32 taken = frameElapsed.AsSeconds() * 1000.0f;
			LOG_E(
				"Game Loop is taking too long! Game time is inaccurate (slowed down) "
				"[max: "
				"%.2fms taken: %.2fms]",
				max, taken);
			logTime = Time::Now();
			consecutive = 0;
		}
	}
	else
	{
		consecutive = 0;
	}
}
#endif

void Sleep(Time time)
{
	if (time.AsMilliseconds() > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(time.AsMilliseconds()));
	}
}

void Cleanup()
{
#if ENABLED(CONSOLE)
	Console::Cleanup();
#endif
#if ENABLED(PROFILER)
	Profiler::Cleanup();
#endif
	uWSM = nullptr;
	uContext = nullptr;
	uAudio = nullptr;
	uShaders = nullptr;
	uRepository = nullptr;
#if !SHIPPING
	config.Save(".game.conf");
#endif
	Core::Jobs::Cleanup();
	LOG_I("[GameLoop] Terminated");
	Core::StopFileLogging();
}
} // namespace

s32 GameLoop::Run(s32 argc, char** argv)
{
	if (!Init(argc, argv))
	{
		LOG_E("[GameLoop] Fatal Error initialising GameLoop!");
		Cleanup();
		return -1;
	}

	CreateContext(config);
	uWSM = MakeUnique<WorldStateMachine>(*uContext);
#if ENABLED(CONSOLE)
	Console::Init(*uContext);
#endif
#if ENABLED(PROFILER)
	Profiler::Init(*uContext, Time::Milliseconds(10));
#endif
	WorldClock::Reset();
	uWSM->Start("Manifest.amf", "Texts/Game.style", &GameInit::LoadShaders);

	Time t;
	Time accumulator;
	Time currentTime = Time::Now();
	while (!uContext->IsTerminating())
	{
		Time frameElapsed;
		uContext->PollInput();
		// Break and exit if Window closed
		if (uContext->IsTerminating())
		{
			break;
		}

		if (!uContext->IsPaused())
		{
			uContext->StartFrame();
			Time dt = tickRate;
			Time newTime = Time::Now();
			Time frameTime = Time::Clamp(newTime - currentTime, Time::Zero, maxFrameTime);
			currentTime = newTime;

			accumulator += frameTime;
			while (accumulator >= dt)
			{
				WorldClock::Tick(dt);
				bool bYield = Tick(dt);
#if ENABLED(CONSOLE)
				Debug::Console::Tick(dt);
#endif
				if (bYield)
				{
					accumulator = Time::Zero;
					LOG_D("[EventLoop] Yielded integration");
					break;
				}
				accumulator -= dt;
			}

			uContext->SubmitFrame();

			if (uContext->IsTerminating())
			{
				break;
			}
#if DEBUGGING
			ProfileFrameTime(Time::Now() - currentTime, tickRate);
#endif
			frameElapsed = Time::Now() - currentTime;
		}
		else
		{
			frameElapsed = tickRate.Scale(Fixed(0.25f));
		}
		Sleep(tickRate - frameElapsed);
	}

	Cleanup();
	return 0;
}
} // namespace LittleEngine