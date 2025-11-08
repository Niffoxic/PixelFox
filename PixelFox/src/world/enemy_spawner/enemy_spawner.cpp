#include "enemy_spawner.h"

#include "enemy/define_enemy.h"
#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/render_manager/render_queue/sampler/sample_allocator.h"
#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/render_manager/render_queue/render_queue.h"

#include <algorithm>
#include <cmath>

using namespace pixel_game;
using namespace pixel_engine;


_Use_decl_annotations_
EnemySpawner::EnemySpawner(PlayerCharacter* player)
	: m_pPlayer(player) 
{}

_Use_decl_annotations_
bool pixel_game::EnemySpawner::Initialize(const PG_SPAWN_DESC& desc)
{
	if (!m_pPlayer || !m_pPlayer->GetPlayerBody())
	{
		logger::error("EnemySpawner::Initialize: Player/body not set!");
		return false;
	}

	m_desc = desc;
	m_elapsedTime = 0.0f;
	m_nSpawnedCount = 0;
	m_initialized = true;

	BuildEnemies();

	//~ Pushing all the enemies in the map
	//~ with visiblity set to false to avoid runtime loading in single thread
	const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();

	int prepared = 0;
	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e) continue;

		//~ to avoid sudden collision
		const FVector2D spawnPos = playerPos + m_spawnOffset * 10.f;

		PG_ENEMY_INIT_DESC init{};
		init.SpawnPoint = spawnPos;
		init.Scale		= { 3.0f, 3.0f };
		init.pTarget	= m_pPlayer->GetPlayerBody();

		if (!e->Initialize(init))
		{
			logger::error("EnemySpawner::Initialize: init failed.");
			m_mapEnemies[e] = false;
			continue;
		}

		e->SetTarget(init.pTarget);

		if (auto* body = e->GetBody())
		{
			pixel_engine::PhysicsQueue::Instance().AddObject(body);
			body->SetVisible(false); //~ to avoid rendering and collisions
		}

		m_mapEnemies[e] = false;
		++prepared;
	}

	logger::info("EnemySpawner: prepared {} / {} enemies (invisible in queues).",
		prepared, m_desc.SpawnMaxCount);
	return true;
}

_Use_decl_annotations_
void pixel_game::EnemySpawner::Update(float deltaTime)
{
	if (deltaTime > 1.0f) return; //~ avoiding 1st call
	if (!m_initialized) return;

	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e) continue;

		if (m_mapEnemies[e])
		{
			e->Update(deltaTime);

			if (e->IsDead())
			{
				DeactivateEnemy(*e);
			}
		}
	}

	m_elapsedTime     += deltaTime;
	m_activationTimer += deltaTime;

	if (m_elapsedTime < m_desc.SpawnStartTime) return;

	//~ ramp 0 to 1
	const float rampT = std::clamp
	(
		(m_elapsedTime - m_desc.SpawnStartTime) /
		m_desc.SpawnRampTime, 0.0f, 1.0f
	);

	const float interval = std::max(
		m_nMinInterval,
		m_nStartInverval * (1.0f - rampT) + m_nMinInterval * rampT
	);

	int activeNow = 0;
	for (auto& enemy : m_pEnemies)
	{
		if (m_mapEnemies[enemy.get()])
			++activeNow;
	}

	const int desiredActive = static_cast<int>(std::floor(m_desc.SpawnMaxCount * rampT));
	const int maxPerTick    = 1 + static_cast<int>(std::floor(2.0f * rampT));

	if (m_activationTimer < interval) return;
	m_activationTimer -= interval;

	int deficit    = std::max(0, desiredActive - activeNow);
	int toActivate = std::min(deficit, maxPerTick);

	if (toActivate == 0) 
		toActivate = (activeNow < m_desc.SpawnMaxCount) ? 1 : 0;
		
	for (int i = 0; i < toActivate; ++i)
	{
		const int idx = PickInactiveIndexRandom();
		if (idx < 0) break;

		ActivateEnemy(*m_pEnemies[static_cast<size_t>(idx)]);
	}
}

void EnemySpawner::Release()
{
	m_pEnemies.clear();
	m_mapEnemies.clear();
	m_initialized = false;
}

void EnemySpawner::Reset()
{
	m_elapsedTime  = 0.0f;
	m_nSpawnedCount = 0;
	m_pEnemies  .clear();
	m_mapEnemies.clear();
}

void pixel_game::EnemySpawner::BuildEnemies()
{
	m_pEnemies  .clear();
	m_mapEnemies.clear();

	const auto& names = RegistryEnemy::GetEnemyNames();
	
	if (names.empty())
	{
		logger::error("RegistryEnemy::GetEnemyNames() is empty!");
		return;
	}

	m_pEnemies.reserve(static_cast<size_t>(m_desc.SpawnMaxCount));

	for (int i = 0; i < m_desc.SpawnMaxCount; ++i)
	{
		const int idx = RandInt(0, static_cast<int>(names.size()) - 1);
		std::unique_ptr<IEnemy> ptr = RegistryEnemy::CreateEnemy(names[idx]);
		
		if (!ptr) continue;

		m_mapEnemies[ptr.get()] = false;
		m_pEnemies.push_back(std::move(ptr));
	}
}

void pixel_game::EnemySpawner::ActivateEnemy(IEnemy& e)
{
	if (!m_pPlayer || !m_pPlayer->GetPlayerBody())
	{
		logger::error("EnemySpawner::ActivateEnemy: Player/body not set!");
		return;
	}

	const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();

	//~ random quadrant around player
	const int dir = RandInt(0, 3);
	FVector2D base = m_spawnOffset;
	switch (dir)
	{
	case 0: base = { +m_spawnOffset.x, +m_spawnOffset.y }; break;
	case 1: base = { -m_spawnOffset.x, +m_spawnOffset.y }; break;
	case 2: base = { +m_spawnOffset.x, -m_spawnOffset.y }; break;
	case 3: base = { -m_spawnOffset.x, -m_spawnOffset.y }; break;
	}

	// a little jitter to avoid same spot
	const FVector2D jitter
	{
		RandFloat(-m_spawnJitter.x, +m_spawnJitter.x),
		RandFloat(-m_spawnJitter.y, +m_spawnJitter.y)
	};

	const FVector2D spawnPos = playerPos + base + jitter;

	if (auto* body = e.GetBody())
	{
		body->SetPosition(spawnPos);
		body->SetVisible(true);

		m_mapEnemies[&e] = true;
		++m_nSpawnedCount;

		logger::debug("EnemySpawner: activated at ({}, {}), total spawned so far = {}",
			spawnPos.x, spawnPos.y, m_nSpawnedCount);
	}
	else
	{
		logger::error("EnemySpawner::ActivateEnemy: null body.");
	}
}

void pixel_game::EnemySpawner::DeactivateEnemy(IEnemy& e)
{
	if (auto* body = e.GetBody())
		body->SetVisible(false);

	m_mapEnemies[&e] = false;
	// TODO: Reset Enemy State
}

int pixel_game::EnemySpawner::PickInactiveIndexRandom()
{
	int inactiveCount = 0;
	for (size_t i = 0; i < m_pEnemies.size(); ++i)
	{
		IEnemy* e = m_pEnemies[i].get();

		if (e && !m_mapEnemies[e])
			++inactiveCount;
	}

	if (inactiveCount == 0) return -1;

	int target = RandInt(0, inactiveCount - 1);

	for (size_t i = 0; i < m_pEnemies.size(); ++i)
	{
		IEnemy* e = m_pEnemies[i].get();
		if (e && !m_mapEnemies[e])
		{
			if (target == 0)
				return static_cast<int>(i);
			--target;
		}
	}
	return -1;
}

//~ helpers
float EnemySpawner::RandFloat(float min, float max)
{
	std::uniform_real_distribution<float> d(min, max);
	return d(m_rng);
}

int EnemySpawner::RandInt(int min, int max)
{
	std::uniform_int_distribution<int> d(min, max);
	return d(m_rng);
}
