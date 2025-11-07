#pragma once

#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/render_manager/render_queue/render_queue.h"

#include "core/unordered_map.h"
#include "core/vector.h"

#include "player/player.h"
#include <random>

#include "enemy/interface_enemy.h"

namespace pixel_game
{
	struct PG_SPAWN_DESC
	{
		float SpawnStartTime{ 5.0f };
		float SpawnInterval{ 10.0f };
		int   SpawnMaxCount{ 200 };
		float SpawnRampTime{ 120.0f };
	};

	class EnemySpawner
	{
	public:
		explicit EnemySpawner(_In_ PlayerCharacter* player);
		~EnemySpawner() = default;

		bool Initialize(_In_ const PG_SPAWN_DESC& desc);
		void Update(_In_ float deltaTime);
		void Release();
		void Reset();

	private:
		void BuildEnemies();
		void SpawnEnemyAtRandomOffscreen();

		float RandFloat(float min, float max);
		int RandInt(int min, int max);

	private:
		PlayerCharacter* m_pPlayer{ nullptr };
		PG_SPAWN_DESC    m_desc{};
		FVector2D		 m_spawnOffset{ 40, 40 };
		FVector2D m_spawnJitter{ 8.f, 8.f };
		float            m_elapsedTime{ 0.0f };
		float            m_timeSinceLastSpawn{ 0.0f };
		int              m_spawnedCount{ 0 };
		bool             m_initialized{ false };
		std::mt19937     m_rng{ std::random_device{}() };

		fox::vector<std::unique_ptr<IEnemy>> m_pEnemies{};
		fox::unordered_map<IEnemy*, bool> m_mapEnemies{};
	};
} // namespace pixel_game
