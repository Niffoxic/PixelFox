// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"
#include <windows.h> 
#include <chrono>
#include <atomic>
#include <sal.h>

namespace pixel_engine
{
	class PFE_API GameClock
	{
	public:
		using Timer = std::chrono::steady_clock;
		using TimePoint = Timer::time_point;
		
		GameClock();
		
		GameClock(_In_ const GameClock&) = delete;
		GameClock(_Inout_ GameClock&&)   = delete;

		GameClock& operator=(_In_ const GameClock&) = delete;
		GameClock& operator=(_Inout_ GameClock&&)   = delete;

		//~ GameClock features
		void ResetTime();
		
		// returns delta time in seconds
		_NODISCARD _Check_return_ float Tick	   ();
		_NODISCARD _Check_return_ float TimeElapsed() const; // total time in secs
		_NODISCARD _Check_return_ float DeltaTime  () const;
	private:
		std::atomic<TimePoint> m_timeStart;
		std::atomic<TimePoint> m_timeLastTick;
	};
} // namespace pixel_engine
