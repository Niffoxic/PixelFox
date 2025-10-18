#pragma once
#include "PixelFoxEngineAPI.h"
#include <string>
#include <sal.h>
#include <windows.h>

#include "pixel_engine/system_manager/interface/interface_singleton.h"

namespace pixel_engine
{
	class PFE_API logger final: public ISingleton<logger, false>
	{
		friend class ISingleton<logger, false>;
		logger()  = default;
		~logger() = default;
	public:
		//~ no copy and move
		logger(_In_ const logger&) = delete;
		logger(_Inout_ logger&&)   = delete;

		logger& operator=(_In_ const logger&) = delete;
		logger& operator=(_Inout_ logger&&)   = delete;

		//~ logging behaviours
		void test_log() 
		{
			MessageBox(nullptr, "Test", "Logger", MB_OK);
		}

	private:

	};
}
