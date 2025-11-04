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
#include <Windows.h>
#include <initializer_list>
#include <cstdint>
#include <algorithm>
#include <sal.h>

#include "pixel_engine/core/interface/interface_inputs.h"

namespace pixel_engine
{
	static constexpr unsigned int MAX_KEYBOARD_INPUTS = 256u;
#define _FOX_VK_VALID _In_range_(0, MAX_KEYBOARD_INPUTS - 1) _Valid_

	enum PFE_API PEKeyboardMode : uint8_t
	{
		None	= 0,
		Ctrl	= 1,
		Shift	= 1 << 1,
		Alt		= 1 << 2,
		Super	= 1 << 3
	};
	
	class PFE_API PEKeyboardInputs final: public IInputHandler
	{
	public:
		 PEKeyboardInputs() noexcept;
		~PEKeyboardInputs() noexcept override = default;

		//~ No Copy or Move
		PEKeyboardInputs(_In_ const PEKeyboardInputs&) = delete;
		PEKeyboardInputs(_Inout_ PEKeyboardInputs&&)   = delete;

		PEKeyboardInputs& operator=(_In_ const PEKeyboardInputs&) = delete;
		PEKeyboardInputs& operator=(_Inout_ PEKeyboardInputs&&)   = delete;

		//~ Inherited via IInputHandler
		_NODISCARD _Check_return_
		std::string GetObjectName() const override;
		
		_NODISCARD _Check_return_ bool Initialize() override;
		_NODISCARD _Check_return_ bool Release   () override;
		
		_Check_return_ _NODISCARD bool ProcessMessage(
			_In_ UINT message,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam) noexcept override;

		void OnFrameBegin(_In_ float deltaTime) noexcept override;
		void OnFrameEnd  ()				   noexcept override;

		//~ Queries
		_Check_return_ _NODISCARD bool IsKeyPressed  (_FOX_VK_VALID int virtualKey) const noexcept;
		_Check_return_ _NODISCARD bool WasKeyPressed (_FOX_VK_VALID int virtualKey) const noexcept;
		_Check_return_ _NODISCARD bool WasKeyReleased(_FOX_VK_VALID int virtualKey) const noexcept;
		
		_Check_return_ _NODISCARD bool WasChordPressed(_FOX_VK_VALID int key,
			_In_ const pixel_engine::PEKeyboardMode& mode = pixel_engine::PEKeyboardMode::None)
			const noexcept;
		
		_Check_return_ _NODISCARD bool WasMultipleKeyPressed(
			_In_reads_(keys.size()) std::initializer_list<int> keys) const noexcept;

	private:
		//~ Internal Helpers
		void ClearAll() noexcept;
		_Check_return_ _NODISCARD bool IsSetAutoRepeat(_In_ LPARAM lParam) noexcept;

		_Check_return_ _NODISCARD bool IsCtrlPressed () const noexcept;
		_Check_return_ _NODISCARD bool IsShiftPressed() const noexcept;
		_Check_return_ _NODISCARD bool IsAltPressed  () const noexcept;
		_Check_return_ _NODISCARD bool IsSuperPressed() const noexcept;

		_Check_return_ _NODISCARD bool IsInside(_FOX_VK_VALID int virtualKey) const noexcept { return virtualKey >= 0 && virtualKey < MAX_KEYBOARD_INPUTS; }

	private:
		bool m_keyDown	  [MAX_KEYBOARD_INPUTS];
		bool m_keyPressed [MAX_KEYBOARD_INPUTS];
		bool m_keyReleased[MAX_KEYBOARD_INPUTS];
	};
}
