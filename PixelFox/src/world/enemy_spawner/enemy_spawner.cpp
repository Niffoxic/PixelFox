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
void pixel_game::EnemySpawner::Update(float deltaTime)
{
	if (deltaTime > 1.0f) return;
	if (!m_bInitialized) return;

	//~ enemy to player calc
	UpdatePlayerNearest();
	UpdatePlayerMostDense();

	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e) continue;

		if (m_mapEnemies[e])
		{
			e->Update(deltaTime);
			if (e->IsDead())
				DeactivateEnemy(*e);
		}
	}

	m_elapsedTime    += deltaTime;
	m_activationTimer += deltaTime;

	if (m_elapsedTime < m_desc.SpawnStartTime) return;
	if (m_bKeepSpawning && m_nSpawnedCount >= m_desc.SpawnMaxCount) return;

	for (;;)
	{
		const float rampT = std::clamp(
			(m_elapsedTime - m_desc.SpawnStartTime) / m_desc.SpawnRampTime, 0.0f, 1.0f
		);

		const float interval = std::max(
			m_nMinInterval,
			m_nStartInverval * (1.0f - rampT) + m_nMinInterval * rampT
		);

		if (m_activationTimer < interval) break;
		m_activationTimer -= interval;

		int activeNow = 0;
		for (auto& enemy : m_pEnemies)
			if (m_mapEnemies[enemy.get()])
				++activeNow;

		int targetActive = static_cast<int>(std::ceil(m_desc.SpawnMaxCount * rampT));
		int deficit = std::max(0, targetActive - activeNow);
		if (deficit == 0 && !(m_bKeepSpawning && m_nSpawnedCount >= m_desc.SpawnMaxCount))
			deficit = (activeNow < static_cast<int>(m_pEnemies.size())) ? 1 : 0;
		if (deficit <= 0) continue;

		int inactiveCount = 0;
		for (auto& uptr : m_pEnemies)
			if (uptr && !m_mapEnemies[uptr.get()])
				++inactiveCount;

		int toActivate = std::min(deficit, inactiveCount);
		if (toActivate <= 0) continue;

		if (m_bKeepSpawning)
		{
			const int remaining = std::max(0, m_desc.SpawnMaxCount - m_nSpawnedCount);
			toActivate = std::min(toActivate, remaining);
			if (toActivate <= 0) continue;
		}

		for (int i = 0; i < toActivate; ++i)
		{
			const int idx = PickInactiveIndexRandom();
			if (idx < 0) break;
			ActivateEnemy(*m_pEnemies[static_cast<size_t>(idx)]);
		}
	}
}

void EnemySpawner::Release()
{
	m_pEnemies  .clear();
	m_mapEnemies.clear();
	m_bInitialized = false;
}

void EnemySpawner::Reset()
{
	m_elapsedTime   = 0.0f;
	m_nSpawnedCount = 0;
	m_pEnemies  .clear();
	m_mapEnemies.clear();
}

void EnemySpawner::Hide()
{
	for (auto& enemy : m_pEnemies)
	{
		if (enemy) enemy->GetBody()->SetVisible(false);
		m_mapEnemies[enemy.get()] = false;
	}
}

void pixel_game::EnemySpawner::StopAtLimit(bool flag)
{
	m_bKeepSpawning = flag;
}

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
	m_bInitialized = true;

	BuildEnemies();

	const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();

	int prepared = 0;
	const int total = static_cast<int>(m_pEnemies.size());

	for (size_t i = 0; i < m_pEnemies.size(); ++i)
	{
		IEnemy* e = m_pEnemies[i].get();
		if (!e) continue;

		//~ Loading screen details
		if (m_desc.pLoadTitle)
		{
			const int humanIdx = static_cast<int>(i) + 1;
			m_desc.pLoadTitle->SetText(
				"Allocating enemy (" + std::to_string(humanIdx) + "/" +
				std::to_string(m_desc.SpawnMaxCount) + ")"
			);
		}

		if (m_desc.pLoadDescription)
		{
			m_desc.pLoadDescription->SetText(
				"creating enemy " + std::string(e->GetTypeName())
			);
		}

		const FVector2D spawnPos = playerPos + m_spawnOffset * 10.f;

		PG_ENEMY_INIT_DESC init{};
		init.SpawnPoint = spawnPos;
		init.Scale   = { 3.0f, 3.0f };
		init.pTarget = m_pPlayer->GetPlayerBody();

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
			body->SetVisible(false);
		}

		m_mapEnemies[e] = false;
		++prepared;
	}

	if (m_desc.pLoadTitle)
		m_desc.pLoadTitle->SetText("Enemies allocated (" + std::to_string(prepared) + "/" + std::to_string(m_desc.SpawnMaxCount) + ")");
	if (m_desc.pLoadDescription)
		m_desc.pLoadDescription->SetText("Initialization complete");

	logger::info("EnemySpawner: prepared {} / {} enemies (invisible in queues).",
		prepared, m_desc.SpawnMaxCount);
	return true;
}

_Use_decl_annotations_
void pixel_game::EnemySpawner::LoadState(const pixel_engine::PEFoxLoader& loader)
{
	const int poolCount = static_cast<int>(m_pEnemies.size());
	const int savedCount = loader.Contains("Count") ? loader["Count"].AsInt() : 0;
	const int count = (std::min)(savedCount, poolCount);

	for (int i = 0; i < poolCount; ++i)
	{
		IEnemy* e = m_pEnemies[i].get();
		if (!e) continue;

		if (m_mapEnemies.contains(e)) m_mapEnemies[e] = false;
		else                          m_mapEnemies[e] = false;

		if (auto* body = e->GetBody())
			body->SetVisible(false);

		e->SetInvisible();
	}

	int activeLoaded = 0;

	for (int i = 0; i < count; ++i)
	{
		IEnemy* e = m_pEnemies[i].get();
		if (!e) continue;

		const std::string key = "E" + std::to_string(i);
		if (!loader.Contains(key)) continue;

		const auto& eNode = loader[key];

		const float px = eNode.Contains("PosX") ? eNode["PosX"].AsFloat() : 0.f;
		const float py = eNode.Contains("PosY") ? eNode["PosY"].AsFloat() : 0.f;

		const bool active = eNode.Contains("Active") ? (eNode["Active"].AsInt() != 0) : false;

		float health = e->GetHealth();
		if (eNode.Contains("Health"))
			health = eNode["Health"].AsFloat();

		if (auto* body = e->GetBody())
		{
			body->SetPosition({ px, py });
			body->SetVisible(active);
		}

		e->SetHealth(health);
		if (m_mapEnemies.contains(e)) m_mapEnemies[e] = active;
		else                          m_mapEnemies[e] = active;

		if (active) ++activeLoaded;
	}

	m_nSpawnedCount = activeLoaded;
	m_activationTimer = 0.0f;
	if (savedCount > poolCount)
	{
		logger::debug("EnemySpawner::LoadState - saved {} exceeds pool {}, truncating.",
			savedCount, poolCount);
	}

	logger::debug("EnemySpawner::LoadState - restored {} active out of {} (pool={})",
		activeLoaded, savedCount, poolCount);
}

_Use_decl_annotations_
void pixel_game::EnemySpawner::SaveState(pixel_engine::PEFoxLoader& enemiesNode)
{
	const int count = static_cast<int>(m_pEnemies.size());
	enemiesNode.GetOrCreate("Count").SetValue(std::to_string(count));

	for (int i = 0; i < count; ++i)
	{
		IEnemy* enemy = m_pEnemies[i].get();
		if (!enemy) continue;

		FVector2D pos{ 40.f, 40.f };
		if (auto* body = enemy->GetBody())
			pos = body->GetPosition();

		bool active = enemy->IsActive();
		if (m_mapEnemies.contains(enemy))
			active = m_mapEnemies[enemy];

		float health = 0.f;
		health = enemy->GetHealth();

		auto& eNode = enemiesNode.GetOrCreate("E" + std::to_string(i));
		eNode.GetOrCreate("PosX").SetValue(std::to_string(pos.x));
		eNode.GetOrCreate("PosY").SetValue(std::to_string(pos.y));
		eNode.GetOrCreate("Active").SetValue(active ? "1" : "0");
		eNode.GetOrCreate("Health").SetValue(std::to_string(health));
	}

	pixel_engine::logger::debug("EnemySpawner::SaveState - saved {} enemies", count);
}

void pixel_game::EnemySpawner::BuildEnemies()
{
	m_pEnemies.clear();
	m_mapEnemies.clear();

	const auto& names = RegistryEnemy::GetEnemyNames();

	if (names.empty())
	{
		logger::error("RegistryEnemy::GetEnemyNames() is empty!");
		return;
	}

	const int totalCount = m_desc.SpawnMaxCount;
	const int enemyTypeCount = static_cast<int>(names.size());
	m_pEnemies.reserve(static_cast<size_t>(totalCount));

	const int baseCount = totalCount / enemyTypeCount;
	const int remainder = totalCount % enemyTypeCount;

	int count = 0;
	for (int t = 0; t < enemyTypeCount; ++t)
	{
		const int spawnCount = baseCount + (t < remainder ? 1 : 0);
		for (int i = 0; i < spawnCount; ++i)
		{
			std::unique_ptr<IEnemy> ptr = RegistryEnemy::CreateEnemy(names[t]);
			if (!ptr) continue;

			m_mapEnemies[ptr.get()] = false;
			m_pEnemies.push_back(std::move(ptr));
			++count;
		}
	}

	logger::info("EnemySpawner::BuildEnemies - Spawned {} enemies ({} types equally divided).", count, enemyTypeCount);
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
	e.Revive();
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
	e.SetInvisible();
	m_mapEnemies[&e] = false;
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

void EnemySpawner::PrepareExistingPoolInvisible_()
{
	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e) continue;

		m_mapEnemies[e] = false;

		e->SetInvisible();
		if (auto* body = e->GetBody())
		{
			body->SetVisible(false);
		}
	}
}

void EnemySpawner::EnsurePoolMatchesDescOrRebuild_()
{
	const size_t want = static_cast<size_t>(m_desc.SpawnMaxCount);
	if (m_pEnemies.size() == want)
	{
		PrepareExistingPoolInvisible_();
		return;
	}

	m_pEnemies.clear();
	m_mapEnemies.clear();

	BuildEnemies();

	if (!m_pPlayer || !m_pPlayer->GetPlayerBody()) return;

	const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();
	int prepared = 0;
	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e) continue;

		const FVector2D spawnPos = playerPos + m_spawnOffset * 10.f;

		PG_ENEMY_INIT_DESC init{};
		init.SpawnPoint = spawnPos;
		init.Scale = { 3.0f, 3.0f };
		init.pTarget = m_pPlayer->GetPlayerBody();

		if (!e->Initialize(init))
		{
			logger::error("EnemySpawner::EnsurePoolMatchesDescOrRebuild_: init failed.");
			m_mapEnemies[e] = false;
			continue;
		}

		e->SetTarget(init.pTarget);

		if (auto* body = e->GetBody())
		{
			pixel_engine::PhysicsQueue::Instance().AddObject(body);
			body->SetVisible(false);
		}

		m_mapEnemies[e] = false;
		++prepared;
	}

	logger::info("EnemySpawner: rebuilt pool; prepared {} / {} enemies (invisible).",
		prepared, m_desc.SpawnMaxCount);
}

void pixel_game::EnemySpawner::UpdatePlayerNearest()
{
	if (!m_pPlayer || !m_pPlayer->IsInitialized())
		return;

	auto* playerBody = m_pPlayer->GetPlayerBody();
	if (!playerBody)
		return;

	const FVector2D playerPos = playerBody->GetRigidBody2D()->GetPosition();

	float nearestDistSq = FLT_MAX;
	FVector2D nearestPos{ 0.f, 0.f };

	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e || !m_mapEnemies[e]) continue;

		auto* enemyBody = e->GetBody();
		if (!enemyBody) continue;

		const FVector2D enemyPos = enemyBody->GetRigidBody2D()->GetPosition();
		const FVector2D delta = enemyPos - playerPos;
		const float distSq = delta.LengthSq();

		if (distSq < nearestDistSq)
		{
			nearestDistSq = distSq;
			nearestPos = enemyPos;
		}
	}

	m_pPlayer->SetNearestTargetLocation(nearestPos);
}

void pixel_game::EnemySpawner::UpdatePlayerMostDense()
{
	if (!m_pPlayer || !m_pPlayer->IsInitialized()) return;
	auto* playerBody = m_pPlayer->GetPlayerBody();
	if (!playerBody) return;

	const float radius = 10.f; 
	FVector2D bestPos{ 0.f, 0.f };
	int bestCount = 0;

	for (auto& uptr : m_pEnemies)
	{
		IEnemy* e = uptr.get();
		if (!e || !m_mapEnemies[e]) continue;
		auto* bodyA = e->GetBody();
		if (!bodyA) continue;
		if(!bodyA->IsVisible()) continue;

		const FVector2D posA = bodyA->GetRigidBody2D()->GetPosition();
		int count = 0;
		FVector2D centroid{ 0.f, 0.f };

		for (auto& inner : m_pEnemies)
		{
			IEnemy* e2 = inner.get();
			if (!e2 || !m_mapEnemies[e2]) continue;
			auto* bodyB = e2->GetBody();
			if (!bodyB) continue;

			const FVector2D posB = bodyB->GetRigidBody2D()->GetPosition();
			if ((posB - posA).LengthSq() <= radius * radius)
			{
				++count;
				centroid += posB;
			}
		}

		if (count > bestCount)
		{
			bestCount = count;
			bestPos = (count > 0) ? (centroid * (1.0f / static_cast<float>(count))) : posA;
		}
	}

	if (bestCount > 0)
		m_pPlayer->SetDenseTargetLocation(bestPos);
}

_Use_decl_annotations_
bool EnemySpawner::Restart()
{
	if (!m_pPlayer || !m_pPlayer->GetPlayerBody())
	{
		logger::error("EnemySpawner::Restart: Player/body not set!");
		return false;
	}

	if (m_pEnemies.empty())
	{
		BuildEnemies();

		const FVector2D playerPos = m_pPlayer->GetPlayerBody()->GetPosition();
		int prepared = 0;
		for (auto& uptr : m_pEnemies)
		{
			IEnemy* e = uptr.get();
			if (!e) continue;

			const FVector2D spawnPos = playerPos + m_spawnOffset * 10.f;

			PG_ENEMY_INIT_DESC init{};
			init.SpawnPoint = spawnPos;
			init.Scale = { 3.0f, 3.0f };
			init.pTarget = m_pPlayer->GetPlayerBody();

			if (!e->Initialize(init))
			{
				logger::error("EnemySpawner::Restart: init failed.");
				m_mapEnemies[e] = false;
				continue;
			}

			e->SetTarget(init.pTarget);

			if (auto* body = e->GetBody())
			{
				pixel_engine::PhysicsQueue::Instance().AddObject(body);
				body->SetVisible(false); 
			}

			m_mapEnemies[e] = false;
			++prepared;
		}

		logger::info("EnemySpawner::Restart: prepared {} / {} enemies (invisible).",
			prepared, m_desc.SpawnMaxCount);
	}
	else
	{
		PrepareExistingPoolInvisible_();
	}

	m_elapsedTime = 0.0f;
	m_activationTimer = 0.0f;
	m_nSpawnedCount = 0;
	m_bInitialized = true;

	logger::info("EnemySpawner: soft restart completed (pool reused).");
	return true;
}
