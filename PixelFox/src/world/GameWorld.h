#pragma once

#include <memory>
#include "player/player.h"
#include "enemy_spawner/enemy_spawner.h"

namespace pixel_game
{
	class GameWorld
	{
	public:
		 GameWorld() = default;
		~GameWorld() = default;

	private:
		std::unique_ptr<PlayerCharacter> m_pPlayer{ nullptr };
	};
}
