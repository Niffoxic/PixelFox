#pragma once
#include "PixelFoxEngineAPI.h"

#include <cstdint>
#include <Windows.h>

#include "core/vector.h"
#include "interface_inputs.h"

namespace pixel_engine
{
	class PFE_API PEMouseInputs final: public IInputHandler
	{
	public:
		 PEMouseInputs();
		~PEMouseInputs() noexcept override = default;

		void AttachWindowHandle(_In_ HWND hWnd);

		//~ Interface Implementation
		_NODISCARD _Check_return_ 
		bool ProcessMessage(
			_In_ UINT message,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam) override;
		
		void OnFrameBegin() noexcept override;
		void OnFrameEnd	 () noexcept override;
		
		//~ Modifiers
		void HideCursor			();
		void UnHideCursor		();
		void LockCursorToWindow	() const;
		void UnlockCursor		() const;

		//~ Queries
		__forceinline
		void GetMousePosition(_In_ int& x, _In_ int& y) const
		{ x = m_pointPosition.x; y = m_pointPosition.y; }
		
		__forceinline
		void GetMouseDelta(_In_ int& dx, _In_ int& dy) const
		{ dx = m_nRawDeltaX; dy = m_nRawDeltaY; }

		_NODISCARD _Check_return_ __forceinline
		bool IsMouseButtonPressed(_In_range_(0, 2) _Valid_ int type) const
		{ return (type >= 0 && type < 3) ? m_bButtonDown[type] : false; }
		
		_NODISCARD _Check_return_ __forceinline
		bool WasButtonPressed	 (_In_range_(0, 2) _Valid_ int type) const
		{ return (type >= 0 && type < 3) ? m_bButtonPressed[type] : false; }
		
		_NODISCARD _Check_return_ __forceinline
		int  GetMouseWheelDelta() const 
		{ return m_nMouseWheelDelta;  }

	private:
		HWND  m_pWindowHandle { nullptr };
		bool  m_bButtonDown	  [3];
		bool  m_bButtonPressed[3];
		POINT m_pointPosition;
		int   m_nRawDeltaX;
		int	  m_nRawDeltaY;
		int   m_nMouseWheelDelta;
		bool  m_bCursorVisible;
	};
} // namespoace pixel_engine
