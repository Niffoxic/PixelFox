#include "pch.h"
#include "clock.h"

pixel_engine::GameClock::GameClock()
{
	ResetTime();
}

void pixel_engine::GameClock::ResetTime()
{
	const TimePoint current = Timer::now();
	
	m_timeStart   .store(current, std::memory_order_seq_cst);
	m_timeLastTick.store(current, std::memory_order_seq_cst);
}

_Use_decl_annotations_
float pixel_engine::GameClock::Tick()
{
    const TimePoint current = Timer::now();

    const TimePoint last = m_timeLastTick.load(std::memory_order_acquire);
    const std::chrono::duration<float> delta = current - last;
    
	m_timeLastTick.store(current, std::memory_order_release);

    return delta.count();
}

_Use_decl_annotations_
float pixel_engine::GameClock::TimeElapsed() const
{
    const TimePoint start   = m_timeStart.load(std::memory_order_acquire);
    const TimePoint current = Timer::now();
    
    return std::chrono::duration<float>(current - start).count();
}

_Use_decl_annotations_
float pixel_engine::GameClock::DeltaTime() const
{
    const TimePoint last    = m_timeLastTick.load(std::memory_order_acquire);
    const TimePoint current = Timer::now();

    return std::chrono::duration<float>(current - last).count();
}
