#include "pch.h"
#include "clock.h"

pixel_engine::clock::clock()
{
	reset_clock();
}

void pixel_engine::clock::reset_clock()
{
	const auto current = Timer::now();
	m_timeStart		   = current;
	m_timeLastTick     = current;
}

_Use_decl_annotations_
float pixel_engine::clock::tick()
{
	const auto current = Timer::now();
	auto delta		   = current - m_timeLastTick;
	m_timeLastTick     = current;

	return delta.count();
}

_Use_decl_annotations_
float pixel_engine::clock::time_elapsed() const
{
	const auto current = Timer::now();
	return std::chrono::duration<float>(current - m_timeStart).count();
}

_Use_decl_annotations_
float pixel_engine::clock::delta() const
{
	const auto current = Timer::now();
	return std::chrono::duration<float>(current - m_timeLastTick).count();
}
