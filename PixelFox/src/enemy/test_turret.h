#pragma once

#include "ai/controller/turret/turret_ai.h"
#include "ai/projectile/straight/straight_projectile.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"

#include <memory>

namespace pixel_game
{
	typedef struct _PG_ENEMY_SPAWN_TEST
	{
		FVector2D SpawnPoint;
		pixel_engine::PEISprite* Target;
	} PG_ENEMY_SPAWN_TEST;

	class TestEnemyTurrent
	{
	public:
		TestEnemyTurrent() = default;
		~TestEnemyTurrent() = default;

		bool Initialize(const PG_ENEMY_SPAWN_TEST& spawnDesc);
		void Update(float deltaTime);
		void Release();

		pixel_engine::PEISprite* GetBody() const;
		pixel_engine::AnimSateMachine* GetAnimState() const;

		IAIController* GetController() const;

	private:
		bool InitializeBody(const PG_ENEMY_SPAWN_TEST& spawnDesc);
		bool InitializeAIController(const PG_ENEMY_SPAWN_TEST& spawnDesc);
		bool InitializeAppearance(const PG_ENEMY_SPAWN_TEST& spawnDesc);
		void UpdateAppearance(float deltaTime);

	private:
		std::unique_ptr<pixel_engine::QuadObject> m_pEnemyBody{ nullptr };
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState{ nullptr };
		std::unique_ptr<TurretAI> m_pAIController{ nullptr };
		std::unique_ptr<StraightProjectile> m_pProjectile{ nullptr };
	};
}
