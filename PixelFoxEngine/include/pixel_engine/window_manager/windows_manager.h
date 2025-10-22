#pragma once

#include <sal.h>
#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_manager.h"

#include "inputs/keyboard_inputs.h"
#include "inputs/mouse_inputs.h"

namespace pixel_engine
{
	//~ Events
	typedef struct _FULL_SCREEN_EVENT
	{
		UINT Width;
		UINT Height;
	} PFE_API FULL_SCREEN_EVENT;

	typedef struct _WINDOWED_SCREEN_EVENT
	{
		UINT Width;
		UINT Height;
	} PFE_API WINDOWED_SCREEN_EVENT;

	typedef struct _WINDOW_RESIZE_EVENT
	{
		UINT Width;
		UINT Height;
	} PFE_API WINDOW_RESIZE_EVENT;

	//~ Event ends
	typedef struct _WINDOW_CREATE_DESC
	{
		std::string WindowTitle;
		_Field_range_(100, 1920) UINT Width;
		_Field_range_(100, 1080) UINT Height;
		_Field_range_(100, 200)  UINT IconId;
	} PFE_API WINDOW_CREATE_DESC;

	class PFE_API PEWindowsManager final: public IManager
	{
	public:
		PEWindowsManager() = default;
		explicit PEWindowsManager(_In_ const WINDOW_CREATE_DESC& desc);
		~PEWindowsManager() override;

		PEWindowsManager(_In_ const PEWindowsManager&) = delete;
		PEWindowsManager(_Inout_ PEWindowsManager&&) = delete;

		PEWindowsManager& operator=(_In_ const PEWindowsManager&) = delete;
		PEWindowsManager& operator=(_Inout_ PEWindowsManager&&) = delete;

		// if returns true means the system gotta shutdown now
		_NODISCARD _Check_return_ static bool ProcessMessage();

		PEKeyboardInputs Keyboard{};
		PEMouseInputs	 Mouse{};

		//~ IManager Interface Implementation
		_NODISCARD _Check_return_ bool OnInit() override;
		_NODISCARD _Check_return_ bool OnRelease() override;

		_NODISCARD _Check_return_ __forceinline
		std::string GetManagerName() const override { return "WindowsManager"; }

		void OnLoopStart(_In_ float deltaTime) override;
		void OnLoopEnd() override;

		//~ Queries
		_NODISCARD _Ret_maybenull_ HWND		 GetWindowsHandle  () const;
		_NODISCARD _Ret_maybenull_ HINSTANCE GetWindowsInstance() const;
		
		_NODISCARD _Check_return_ __forceinline
		bool IsFullScreen() const { return m_bFullScreen; }
		
		void SetFullScreen(_In_ bool flag);

		_NODISCARD _Check_return_ float GetAspectRatio() const;
		
		_NODISCARD _Check_return_ __forceinline
		int GetWindowsWidth () const { return m_nWindowsWidth; }
		_NODISCARD _Check_return_ __forceinline
		int GetWindowsHeight() const { return m_nWindowsHeight; }

		void SetWindowTitle(_In_ const std::string& title) const;

	private:
		_NODISCARD _Check_return_ _Must_inspect_result_
		_Success_(return != 0)
		bool InitWindowScreen();
		
		_NODISCARD
		LRESULT MessageHandler(
			_In_ HWND   hwnd,
			_In_ UINT   msg,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam);

		_Function_class_(WINDOWS_CALLBACK)
		static LRESULT CALLBACK WindowProcSetup(
			_In_ HWND   hwnd,
			_In_ UINT   msg,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam);

		_Function_class_(WINDOWS_CALLBACK)
		static LRESULT CALLBACK WindowProcThunk(
			_In_ HWND   hwnd,
			_In_ UINT   msg,
			_In_ WPARAM wParam,
			_In_ LPARAM lParam);

		void TransitionToFullScreen();
		void TransitionToWindowedScreen() const;

	private:
		std::string		m_szWindowTitle	  {		"Fox Pixel The Game"	};
		std::string		m_szClassName	  {			"FoxGameEngine"		};
		UINT			m_nWindowsWidth   {				640u			};
		UINT			m_nWindowsHeight  {				360u			};
		HWND			m_pWindowsHandle  {				nullptr			};
		HINSTANCE		m_pWindowsInstance{				nullptr			};
		bool			m_bFullScreen	  {				false			};
		WINDOWPLACEMENT m_WindowPlacement { sizeof(m_WindowPlacement)	};
		UINT			m_nIconID		  {				0u				};
	};
} // namespace pixel_engine
