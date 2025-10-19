#pragma once

#include "PixelFoxEngineAPI.h"
#include <windows.h>
#include <sal.h>

namespace pixel_engine
{
	class __declspec(novtable) PFE_API IInputHandler
	{
	public:
		virtual ~IInputHandler() noexcept = default;

		_NODISCARD _Check_return_
			virtual bool ProcessMessage(
				_In_ UINT   message,
				_In_ WPARAM wParam,
				_In_ LPARAM lParam) = 0;

		virtual void OnFrameBegin() noexcept = 0;
		virtual void OnFrameEnd() noexcept = 0;
	};
}
