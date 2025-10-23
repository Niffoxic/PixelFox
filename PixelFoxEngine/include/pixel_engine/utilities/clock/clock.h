#pragma once

#include "PixelFoxEngineAPI.h"
#include <chrono>
#include <sal.h>

namespace pixel_engine
{
	class PFE_API clock
	{
	public:
		using Timer = std::chrono::high_resolution_clock;
		clock();
		
		clock(_In_ const clock&) = delete;
		clock(_Inout_ clock&&)   = delete;

		clock& operator=(_In_ const clock&) = delete;
		clock& operator=(_Inout_ clock&&)   = delete;

		//~ clock features
		void reset_clock();
		
		// returns delta time in seconds
		_NODISCARD _Check_return_ float tick		();
		_NODISCARD _Check_return_ float time_elapsed() const; // total time in secs
		_NODISCARD _Check_return_ float delta	    () const;
	private:
		Timer::time_point m_timeStart;
		Timer::time_point m_timeLastTick;
	};
} // namespace pixel_engine
