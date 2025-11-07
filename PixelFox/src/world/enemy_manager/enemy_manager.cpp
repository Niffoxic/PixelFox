#include "enemy_manager.h"

#include "enemy/define_enemy.h"
#include "pixel_engine/utilities/logger/logger.h"

#include <algorithm>
#include <cmath>

using namespace pixel_game;
using namespace pixel_engine;

_Use_decl_annotations_
EnemySpawner::EnemySpawner(PlayerCharacter* player)
	: m_pPlayer(player)
{
}

_Use_decl_annotations_
bool pixel_game::EnemySpawner::Initialize(const PG_SPAWN_DESC& desc)
{
	if (!m_pPlayer) return false;

	m_desc				 = desc;
	m_elapsedTime		 = 0.0f;
	m_timeSinceLastSpawn = 0.0f;
	m_spawnedCount		 = 0;
	m_initialized		 = true;

	BuildEnemies();

	return true;
}

_Use_decl_annotations_
void pixel_game::EnemySpawner::Update(float deltaTime)
{
	if (!m_initialized) return;

	for (auto& enemy : m_pEnemies)
	{
		if (m_mapEnemies[enemy.get()])
			enemy->Update(deltaTime);
	}

	m_elapsedTime += deltaTime;
	m_timeSinceLastSpawn += deltaTime;

	if (m_elapsedTime < m_desc.SpawnStartTime) return;
	if (m_spawnedCount >= m_desc.SpawnMaxCount) return;

	const float spawnFreq = std::clamp(
		(m_elapsedTime - m_desc.SpawnStartTime) / m_desc.SpawnRampTime, 0.0f, 1.0f
	);

	const float minInterval = 0.35f;
	const float interval = std::max(
		minInterval,
		m_desc.SpawnInterval * (1.0f - spawnFreq) + minInterval * spawnFreq
	);

	if (m_timeSinceLastSpawn < interval) return;
	m_timeSinceLastSpawn = 0.0f;

	SpawnEnemyAtRandomOffscreen();
}

void EnemySpawner::Release()
{
	m_pEnemies.clear();
	m_initialized = false;
}

void EnemySpawner::Reset()
{
	m_elapsedTime = 0.0f;
	m_timeSinceLastSpawn = 0.0f;
	m_spawnedCount = 0;
	m_pEnemies.clear();
}

void pixel_game::EnemySpawner::BuildEnemies()
{
	m_pEnemies.clear();
	m_mapEnemies.clear();

	const auto& names = RegistryEnemy::GetEnemyNames();
	if (names.empty())
	{
		pixel_engine::logger::error("RegistryEnemy::GetEnemyNames() is empty!");
		return;
	}

	m_pEnemies.reserve(static_cast<size_t>(m_desc.SpawnMaxCount));

	for (int i = 0; i < m_desc.SpawnMaxCount; ++i)
	{
		const int idx = RandInt(0, static_cast<int>(names.size()) - 1);
		const std::string& chosen = names[idx];

		std::unique_ptr<IEnemy> ptr = RegistryEnemy::CreateEnemy(chosen);
		if (!ptr) continue;

		m_mapEnemies[ptr.get()] = false;
		m_pEnemies.push_back(std::move(ptr));
	}
}

void pixel_game::EnemySpawner::SpawnEnemyAtRandomOffscreen()
{
	using pixel_engine::logger;

	auto* pCamera = pixel_engine::PERenderQueue::Instance().GetCamera();
	if (!pCamera)
	{
		logger::error("EnemySpawner::SpawnEnemyAtRandomOffscreen: Camera not found!");
		return;
	}
	if (!m_pPlayer || !m_pPlayer->GetPlayerBody())
	{
		logger::error("EnemySpawner::SpawnEnemyAtRandomOffscreen: Player/body not set!");
		return;
	}

	const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();

	const int dir = RandInt(0, 3);

	FVector2D offset = m_spawnOffset;
	switch (dir)
	{
	case 0: offset = { +m_spawnOffset.x, +m_spawnOffset.y }; break; // +x +y
	case 1: offset = { -m_spawnOffset.x, +m_spawnOffset.y }; break; // -x +y
	case 2: offset = { +m_spawnOffset.x, -m_spawnOffset.y }; break; // +x -y
	case 3: offset = { -m_spawnOffset.x, -m_spawnOffset.y }; break; // -x -y
	}

	const FVector2D jitter{
		RandFloat(-m_spawnJitter.x, m_spawnJitter.x),
		RandFloat(-m_spawnJitter.y, m_spawnJitter.y)
	};

	const FVector2D spawnPos = { playerPos.x + offset.x + jitter.x,
								 playerPos.y + offset.y + jitter.y };

	logger::debug("EnemySpawner: Spawning near player at ({}, {}) | dir={} | offset=({}, {}) | jitter=({}, {})",
		spawnPos.x, spawnPos.y, dir, offset.x, offset.y, jitter.x, jitter.y);

	IEnemy* chosen = nullptr;
	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (m_mapEnemies.contains(e) && !m_mapEnemies[e])
		{
			chosen = e;
			break;
		}
	}
	if (!chosen)
	{
		logger::debug("EnemySpawner: No inactive enemies available to spawn.");
		return;
	}
	logger::debug("EnemySpawner: Found inactive enemy instance {}", (void*)chosen);

	PG_ENEMY_INIT_DESC init{};
	init.SpawnPoint = spawnPos;
	init.Scale = { 3.0f, 3.0f };
	init.pTarget = m_pPlayer->GetPlayerBody();

	if (!chosen->Initialize(init))
	{
		logger::error("EnemySpawner: Enemy initialization failed at ({}, {})", spawnPos.x, spawnPos.y);
		return;
	}
	logger::debug("EnemySpawner: Enemy initialized at ({}, {})", spawnPos.x, spawnPos.y);

	chosen->SetTarget(init.pTarget);

	auto* body = chosen->GetBody();
	if (!body)
	{
		logger::error("EnemySpawner: Enemy body is null after initialization!");
		return;
	}
	pixel_engine::PhysicsQueue::Instance().AddObject(body);
	logger::debug("EnemySpawner: Added enemy body {} to PhysicsQueue", (void*)body);

	m_mapEnemies[chosen] = true;
	++m_spawnedCount;

	logger::debug("EnemySpawner: Enemy activated. Total spawned = {}", m_spawnedCount);
}

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
