#pragma once

#include <memory>
#include "player/player.h"
#include "enemy_spawner/enemy_spawner.h"
#include "pixel_engine/render_manager/components/font/font.h"
#include "pixel_engine/window_manager/inputs/keyboard_inputs.h"
#include "menu_gui/main_menu.h"

namespace pixel_game
{
	typedef struct _PG_GAME_WORLD_CONSTRUCT_DESC
	{
		pixel_engine::PEKeyboardInputs* pKeyboard;
		pixel_engine::PEWindowsManager* pWindowsManager;
	} PG_GAME_WORLD_CONSTRUCT_DESC;
	class GameWorld
	{
		enum class GameState
		{
			Menu,
			Finite,
			Infinite,
		};

	public:
		 GameWorld(const PG_GAME_WORLD_CONSTRUCT_DESC& desc);
		~GameWorld();

		void Update(float deltaTime);

	private:
		//~ build
		void BuildMainMenu(const PG_GAME_WORLD_CONSTRUCT_DESC& desc);

		void AttachCameraToPlayer() const;

		//~ Handle State Transition
		void UpdateActiveState(float deltaTime);
		void HandleTransition ();

		void SetState  (GameState next);
		void EnterState(GameState state);
		void ExitState (GameState state);

		//~ watch keys
		void KeyWatcher(float deltaTime);

		//~ meta info
		void BuildFPSFont();
		void ComputeFPS(float deltaTime);
		void ShowFPS();
	private:
		//~ systems
		pixel_engine::PEWindowsManager* m_pWindows { nullptr };
		pixel_engine::PEKeyboardInputs* m_pKeyboard{ nullptr };
		float m_nInputBlockTimer{ 0.0f };
		float m_nInputDelay		{ 0.2f };

		//~ displays fps
		std::unique_ptr<pixel_engine::PEFont> m_fps{ nullptr };
		bool  m_bShowFPS    { false };
		int   m_nFrameCount { 0 };
		int   m_nLastFps    { 0 };
		float m_nTimeElapsed{ 0.0f };

		GameState m_eGameState    { GameState::Menu };
		GameState m_ePrevGameState{ GameState::Menu };

		std::unique_ptr<MainMenu>		 m_pMainMenu	{ nullptr };
		std::unique_ptr<PlayerCharacter> m_pPlayer	    { nullptr };
		std::unique_ptr<EnemySpawner>    m_pEnemySpawner{ nullptr };

		//~ helper fonts
		std::unique_ptr<pixel_engine::PEFont> m_pFPSFont		  { nullptr };
		std::unique_ptr<pixel_engine::PEFont> m_pDetailsFont	  { nullptr };
		std::unique_ptr<pixel_engine::PEFont> m_pLevelCompleteFont{ nullptr };
	};
}
