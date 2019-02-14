#include "stdafx.h"
#include <functional>
#include "AsyncRenderLoop.h"
#include "RenderHeap.h"
#include "LittleEngine/Services/Services.h"
#include "Asserts.h"
#include "Logger.h"
#include "SFMLAPI/Rendering/GFXBuffer.h"
#include "SFMLAPI/Windowing/SFWindow.h"
#include "SFML/Graphics.hpp"

namespace LittleEngine
{
AsyncRenderLoop::AsyncRenderLoop(SFWindow& glWindow, GFXBuffer& gfxBuffer, Time tickRate, bool bStartThread)
	: SFRenderer(glWindow, tickRate), m_pBuffer(&gfxBuffer)
{
	if (bStartThread)
	{
		LogI("[AsyncRenderLoop] Deactivating GLWindow on this thread, starting render thread");
		m_pSFWindow->setActive(false);
		m_renderThreadJobID = Services::Jobs()->EnqueueSystem(
			std::bind(&AsyncRenderLoop::Run, this), "Async Render Loop");
	}
}

AsyncRenderLoop::~AsyncRenderLoop()
{
	m_bRendering = false;
	if (m_renderThreadJobID != JobManager::INVALID_ID)
		Services::Jobs()->Wait(m_renderThreadJobID);
}

void AsyncRenderLoop::Run()
{
	LogI("R[AsyncRenderLoop] Activating GLWindow on this thread");
	m_pSFWindow->setActive(true);
	m_pSFWindow->setVerticalSyncEnabled(true);
	while (m_bRendering)
	{
		Assert(m_pSFWindow, "[AsyncRenderLoop] SFML RenderWindow is nullptr!");

		Render(*m_pBuffer);
		Display();
	}
	LogI("R[AsyncRenderLoop] Deactivating GLWindow on this thread, terminating job");
	m_pSFWindow->setActive(false);
}
} // namespace LittleEngine
