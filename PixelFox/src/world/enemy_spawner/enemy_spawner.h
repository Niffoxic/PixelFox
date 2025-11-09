#pragma once

#include "core/unordered_map.h"
#include "core/vector.h"

#include "player/player.h"
#include "enemy/interface_enemy.h"
#include "pixel_engine/render_manager/components/font/font.h"

#include <random>
#include "pixel_engine/utilities/fox_loader/fox_loader.h"

namespace pixel_game
{
	struct PG_SPAWN_DESC
	{
		float SpawnStartTime{ 5.0f };
		int   SpawnMaxCount{ 200 }; 
		float SpawnRampTime{ 60.f };
		_In_ pixel_engine::PEFont* pLoadTitle;
		_In_ pixel_engine::PEFont* pLoadDescription;
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
		
		void Hide();

		bool IsInitialized() const { return m_bInitialized; }

		void StopAtLimit(bool flag); // if set to false enemies will keep spawnning

		_Success_(return != false)
		bool Restart();

		//~ save and load state
		void LoadState(_In_ const pixel_engine::PEFoxLoader& loader);
		void SaveState(_In_ pixel_engine::PEFoxLoader& loader);

	private:
		void BuildEnemies();               
		void ActivateEnemy(IEnemy& e);   
		void DeactivateEnemy(IEnemy& e);   
		int  PickInactiveIndexRandom();  

		float RandFloat(float min, float max);
		int   RandInt(int min, int max);

		//~ helpers
		void PrepareExistingPoolInvisible_();
		void EnsurePoolMatchesDescOrRebuild_();

	private:
		PlayerCharacter* m_pPlayer{ nullptr };
		PG_SPAWN_DESC    m_desc{};

		FVector2D m_spawnOffset{ 20.f, 20.f };
		FVector2D m_spawnJitter{ 8.f, 8.f };

		bool  m_bKeepSpawning{ false };
		float m_activationTimer{ 0.0f };
		float m_elapsedTime    { 0.0f };
		int   m_nSpawnedCount   { 0 };
		float m_nStartInverval{ 3.f };
		float m_nMinInterval{ 0.35f };
		bool  m_bInitialized{ false };


		std::mt19937 m_rng{ std::random_device{}() };

		fox::vector<std::unique_ptr<IEnemy>> m_pEnemies{};
		fox::unordered_map<IEnemy*, bool>    m_mapEnemies{};

	};
} // namespace pixel_game
