#pragma once
#include "PixelFoxEngineAPI.h"
#include "pixel_engine/system_manager/interface/interface_manager.h"

#include "inputs/keyboard_inputs.h"
#include "inputs/mouse_inputs.h"

namespace pixel_engine
{
	typedef struct _WINDOW_CREATE_DESC
	{
		std::string WindowTitle;
		UINT		Width;
		UINT		Height;
	} PFE_API WINDOW_CREATE_DESC;

	class PFE_API PEWindowsManager final: public IManager
	{
	public:
		PEWindowsManager() = default;
		explicit PEWindowsManager(const WINDOW_CREATE_DESC& desc);
		~PEWindowsManager() override;

		PEWindowsManager(const PEWindowsManager&) = delete;
		PEWindowsManager(PEWindowsManager&&) = delete;

		PEWindowsManager& operator=(const PEWindowsManager&) = delete;
		PEWindowsManager& operator=(PEWindowsManager&&) = delete;

		// if returns true means the system gotta shutdown now
		static bool ProcessMessage();

		PEKeyboardInputs Keyboard{};
		PEMouseInputs	 Mouse{};

		//~ IManager Interface Implementation
		_NODISCARD _Check_return_ bool OnInit() override;
		_NODISCARD _Check_return_ bool OnRelease() override;

		_NODISCARD _Check_return_ __forceinline
		std::string GetManagerName() const override { return "WindowsManager"; }

		void OnLoopStart(float deltaTime) override;
		void OnLoopEnd() override;

		//~ Queries
		HWND GetWindowsHandle() const;
		HINSTANCE GetWindowsInstance() const;

		bool IsFullScreen() const { return m_bFullScreen; }
		void SetFullScreen(bool flag);

		float GetAspectRatio() const;
		
		int GetWindowsWidth() const  { return m_nWindowsWidth; }
		int GetWindowsHeight() const { return m_nWindowsHeight; }

		void SetWindowTitle(const std::string& title) const;

	private:
		bool InitWindowScreen();
		
		LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static LRESULT CALLBACK WindowProcSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	
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
	};
} // namespace pixel_engine
