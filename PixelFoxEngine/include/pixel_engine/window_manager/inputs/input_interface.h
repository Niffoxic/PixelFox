#pragma once

#include <windows.h>
#include <sal.h>

__interface  IInputHandler 
{
	_NODISCARD _Check_return_
	bool ProcessMessage(
		_In_ UINT   message,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam);

	void OnFrameBegin() noexcept;
	void OnFrameEnd  () noexcept;
};
