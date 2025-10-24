#pragma once

#include "PixelFoxEngineAPI.h"

#include <windows.h>
#include <sal.h>

#include "interface_frame.h"

namespace pixel_engine
{
	class PFE_API IInputHandler: public IFrameObject
	{
	public:
		_NODISCARD _Check_return_
			virtual bool ProcessMessage(
				_In_ UINT   message,
				_In_ WPARAM wParam,
				_In_ LPARAM lParam) = 0;
	};
}
