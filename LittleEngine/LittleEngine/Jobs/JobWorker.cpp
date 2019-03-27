#include "stdafx.h"
#include "Logger.h"
#include "Utils.h"
#include "JobWorker.h"
#include "JobManager.h"

namespace LittleEngine
{
JobWorker::JobWorker(JobManager& manager, u8 id, bool bEngineWorker)
	: id(id), m_pManager(&manager), m_bEngineWorker(bEngineWorker)
{
	m_bWork.store(true, std::memory_order_relaxed);
	m_logName = "[JobWorker" + Strings::ToString(this->id) + "]";
	m_thread = std::thread(std::bind(&JobWorker::Run, this));
}

JobWorker::~JobWorker()
{
	if (m_thread.joinable())
	{
		m_thread.join();
	}
	LOG_D("%s destroyed", m_logName.c_str());
}

void JobWorker::Stop()
{
	m_bWork.store(false, std::memory_order_relaxed);
}

JobWorker::State JobWorker::GetState() const
{
	return m_state;
}

void JobWorker::Run()
{
	while (m_bWork.load(std::memory_order_relaxed))
	{
		// Reset
		m_state = State::IDLE;

		// Obtain job
		UPtr<JobManager::Job> uJob = m_pManager->Lock_PopJob(m_bEngineWorker);

		// No jobs in queue
		if (!uJob)
		{
			std::this_thread::yield();
		}

		// Perform job
		else
		{
			m_state = State::WORKING;

			String suffix = m_bEngineWorker ? " Engine Job " : " Job ";
			if (!uJob->m_bSilent)
				LOG_D("%s Starting %s %s", m_logName.c_str(),
					  m_bEngineWorker ? "Engine Job" : "Job", uJob->ToStr());
			// TODO: Retrieve and defer any exceptions thrown
			uJob->Run();
			if (!uJob->m_bSilent)
				LOG_D("%s Completed %s %s", m_logName.c_str(),
					  m_bEngineWorker ? "Engine Job" : "Job", uJob->ToStr());
			uJob->Fulfil();
		}
	}
}
} // namespace LittleEngine
