#include "pch.h"
#include "raster_scheduler.h"

pixel_engine::RasterizeScheduler::~RasterizeScheduler()
{
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_Stop = true;
    }
    m_Condition.notify_all();

    for (auto& t : m_WorkerThreads)
    {
        if (t.joinable())
            t.join();
    }

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_TaskQueue.clear();
        m_ActiveTasks = 0;
    }
}

void pixel_engine::RasterizeScheduler::Initialize(std::uint32_t workerCount)
{
    if (workerCount == 0)
        workerCount = std::max(1u, std::thread::hardware_concurrency());

    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        m_Stop = false;
    }

    m_WorkerThreads.reserve(workerCount);
    for (std::uint32_t i = 0; i < workerCount; ++i)
    {
        m_WorkerThreads.emplace_back([this, i]()
            {
                WorkerLoop(i);
            });
    }
}

void pixel_engine::RasterizeScheduler::Enqueue(const PERasterizeTask& task)
{
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        if (m_Stop)
            return;

        m_TaskQueue.push_back(task);
        ++m_ActiveTasks;
    }
    m_Condition.notify_one();
}

void pixel_engine::RasterizeScheduler::Enqueue(PERasterizeTask&& task)
{
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        if (m_Stop)
            return;

        m_TaskQueue.emplace_back(std::move(task));
        ++m_ActiveTasks;
    }
    m_Condition.notify_one();
}

void pixel_engine::RasterizeScheduler::Dispatch()
{
    MasterWork();
}

void pixel_engine::RasterizeScheduler::Wait()
{
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    m_Condition.wait(lock, [this]()
    {
        return (m_TaskQueue.empty() && (m_ActiveTasks.load() == 0));
    });
}

void pixel_engine::RasterizeScheduler::WorkerLoop(std::uint32_t workerIndex)
{
    for (;;)
    {
        PERasterizeTask task;

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_Condition.wait(lock, [this]()
                {
                    return m_Stop || !m_TaskQueue.empty();
                });

            if (m_Stop && m_TaskQueue.empty())
                return;

            task = std::move(m_TaskQueue.back());
            m_TaskQueue.pop_back();
        }

        task.Execute();

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            if (m_ActiveTasks.fetch_sub(1) == 1)
            {
                m_Condition.notify_all();
            }
        }
    }
}

void pixel_engine::RasterizeScheduler::MasterWork()
{
    for (;;)
    {
        PERasterizeTask task;

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            if (m_TaskQueue.empty() || m_Stop)
                break;

            task = std::move(m_TaskQueue.back());
            m_TaskQueue.pop_back();
        }

        task.Execute();

        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            if (m_ActiveTasks.fetch_sub(1) == 1)
            {
                m_Condition.notify_all();
            }
        }
    }
}
