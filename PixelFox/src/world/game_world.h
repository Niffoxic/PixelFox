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
	public:
		 GameWorld(const PG_GAME_WORLD_CONSTRUCT_DESC& desc);
		~GameWorld();

		void Update(float deltaTime);

	private:
		void AttachCameraToPlayer() const;

	private:
		std::unique_ptr<MainMenu>		 m_pMainMenu	{ nullptr };
		std::unique_ptr<PlayerCharacter> m_pPlayer	    { nullptr };
		std::unique_ptr<EnemySpawner>    m_pEnemySpawner{ nullptr };

		//~ helper fonts
		std::unique_ptr<pixel_engine::PEFont> m_pFPSFont		  { nullptr };
		std::unique_ptr<pixel_engine::PEFont> m_pDetailsFont	  { nullptr };
		std::unique_ptr<pixel_engine::PEFont> m_pLevelCompleteFont{ nullptr };
	};
}
