#pragma once

#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"
#include "pixel_engine/window_manager/windows_manager.h"
#include "pixel_engine/render_manager/components/font/font.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"

#include "core/vector.h"

#include <functional>
#include <string>
#include <sal.h>

namespace pixel_game
{
	typedef struct _PG_MAIN_MENU_DESC
	{
		pixel_engine::PEKeyboardInputs* pKeyboard;
		pixel_engine::PEWindowsManager* pWindows;
		std::function<void()> OnEnterPress_FiniteMap;
		std::function<void()> OnEnterPress_InfiniteMap;
		std::function<void()> OnEnterPress_Quit;
	} PG_MAIN_MENU_DESC;

	class MainMenu 
	{
	public:
		explicit MainMenu(const PG_MAIN_MENU_DESC& desc);
		~MainMenu();

		MainMenu(const MainMenu&) = delete;
		MainMenu(MainMenu&&)	  = delete;

		MainMenu& operator=(const MainMenu&) = delete;
		MainMenu& operator=(MainMenu&&)		 = delete;

		void Init();
		void Update(float deltaTime);

		void Show();
		void Hide();

		_NODISCARD _Check_return_
		bool IsVisible() const;

	private:
		void HandleInput(float deltaTime);

		//~ construct
		void BuildLayout	    ();
		void BuildFonts		    ();
		void BuildControlsLayout();

		//~ show
		void ShowCommonLayout();
		void HideCommonLayout();

		void ShowMainMenu    ();
		void HideMainMenu	 ();
		void ShowControlsMenu();
		void HideControlsMenu();

		// selection helpers
		void SelectNext();
		void SelectPrev();
		void ActivateSelected();

	private:
		enum class EMenuState : int { Main = 0, Controls };

		static constexpr int m_nItem_Finite   = 0;
		static constexpr int m_nItem_Infinite = 1;
		static constexpr int m_nItem_Controls = 2;
		static constexpr int m_nItem_Quit	  = 3;
		static constexpr int m_ItemCount	  = 4;

		// state
		float		m_nInputDelay{ 0.f };
		EMenuState  m_eState{ EMenuState::Main };
		bool        m_bVisible{ true };
		int         m_nSelectedIndex{ m_nItem_Finite };

		//~ operation mem
		pixel_engine::PEKeyboardInputs* m_pKeyboard{ nullptr };
		pixel_engine::PEWindowsManager* m_pWindows{ nullptr };

		//~ backgrounds
		pixel_engine::QuadObject				m_menuBackground;
		std::unique_ptr<pixel_engine::AnimSateMachine>	m_pBackgroundAnim{nullptr};
		pixel_engine::QuadObject				m_menuScreen;
		fox::vector<std::unique_ptr<pixel_engine::PEFont>>		m_menuTexts;

		//~ options
		pixel_engine::QuadObject m_menuFiniteMap;
		pixel_engine::QuadObject m_menuHoverFiniteMap;
		pixel_engine::PEFont	 m_menuFiniteMapText;

		pixel_engine::QuadObject m_menuInfiniteMap;
		pixel_engine::QuadObject m_menuHoverInfiniteMap;
		pixel_engine::PEFont	 m_menuInfiniteMapText;

		pixel_engine::QuadObject m_menuControlsOption;
		pixel_engine::QuadObject m_menuHoverControlsOption;
		pixel_engine::PEFont	 m_menuControlsOptionText;

		pixel_engine::QuadObject m_menuQuitOption;
		pixel_engine::QuadObject m_menuHoverQuitOption;
		pixel_engine::PEFont	 m_menuQuitOptionText;

		// settings page
		pixel_engine::QuadObject		  m_controlsBackground;
		pixel_engine::QuadObject		  m_controlsPanel;
		fox::vector<pixel_engine::PEFont> m_controlsPanelTexts{};

		//~ callbacks
		std::function<void()> m_fnOnEnterPress_FiniteMap;
		std::function<void()> m_fnOnEnterPress_InfiniteMap;
		std::function<void()> m_fnOnEnterPress_Quit;
	};
} // namespace pixel_game
