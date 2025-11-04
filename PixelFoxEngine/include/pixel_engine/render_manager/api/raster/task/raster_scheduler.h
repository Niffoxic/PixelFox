#pragma once

#include "PixelFoxEngineAPI.h"
#include "raster_task.h"

#include <thread>
#include <vector>
#include <atomic>
#include <memory>
#include <condition_variable>
#include <mutex>

namespace pixel_engine
{
	class PFE_API RasterizeScheduler
	{
	public:
		RasterizeScheduler() noexcept = default;
		~RasterizeScheduler();

		//~ Initialization & shutdown
		void Initialize(std::uint32_t workerCount = std::thread::hardware_concurrency());
		void Shutdown();

		//~ Job submission
		void Enqueue(const PERasterizeTask& task);
		void Enqueue(PERasterizeTask&& task);

		//~ Execution control
		void Dispatch();   // Divide work (Master logic)
		void Wait();

	private:
		void WorkerLoop(std::uint32_t workerIndex);
		void MasterWork();

	private:
		std::vector<std::thread> m_WorkerThreads;
		std::vector<PERasterizeTask> m_TaskQueue;

		std::mutex m_QueueMutex;
		std::condition_variable m_Condition;

		std::atomic<bool> m_Stop{ false };
		std::atomic<std::size_t> m_ActiveTasks{ 0 };
	};

} // namespace pixel_engine
