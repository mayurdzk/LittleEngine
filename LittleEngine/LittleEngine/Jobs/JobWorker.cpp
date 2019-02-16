#include "stdafx.h"
#include "JobWorker.h"
#include "JobManager.h"
#include "Logger.h"
#include "Utils.h"

namespace LittleEngine
{
using Lock = std::lock_guard<std::mutex>;

JobWorker::JobWorker(JobManager& manager, u32 id, bool bSystemWorker)
	: id(id), m_pManager(&manager), m_bSystemWorker(bSystemWorker)
{
	m_logName = "[JobWorker" + Strings::ToString(id);
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
	m_bWork.store(false);
}

void JobWorker::Wait()
{
	while (m_state == State::WORKING)
	{
		std::this_thread::yield();
	}
}

JobID JobWorker::GetJobID() const
{
	return m_jobID;
}

JobWorker::State JobWorker::GetState() const
{
	return m_state;
}

void JobWorker::Run()
{
	while (m_bWork.load())
	{
		// Reset
		m_state = State::IDLE;
		m_jobID = JobManager::INVALID_ID;

		// Obtain job
		JobManager::Job job;
		{
			Lock lock(m_pManager->m_mutex);
			List<JobManager::Job>& jobList =
				m_bSystemWorker ? m_pManager->m_systemJobQueue : m_pManager->m_userJobQueue;
			if (!jobList.empty())
			{
				job = std::move(jobList.back());
				jobList.pop_back();
			}
		}

		// No jobs in queue
		if (job.id == JobManager::INVALID_ID)
		{
			std::this_thread::yield();
		}

		// Perform job
		else
		{
			m_state = State::WORKING;
			m_jobID = job.id;
			String suffix = m_bSystemWorker ? " System Job " : " Job ";
			LOG_D("%s Starting %s %s", m_logName.c_str(), m_bSystemWorker ? "System Job" : "Job", job.ToString());
			job.task();
			{
				Lock lock(m_pManager->m_mutex);
				m_pManager->m_completed.push_back(m_jobID);
			}
			LOG_D("%s Completed %s %s", m_logName.c_str(), m_bSystemWorker ? "System Job" : "Job", job.ToString());
		}
	}
}
} // namespace LittleEngine
