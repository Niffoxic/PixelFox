#pragma once

#include "ai/controller/chase/chase_ai.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"

#include <memory>

namespace pixel_game
{
	typedef struct _PG_ENEMY_SPAWN
	{
		FVector2D SpawnPoint;
		pixel_engine::PEISprite* Target;
	} PG_ENEMY_SPAWN;

	class TestEnemy
	{
	public:
		 TestEnemy() = default;
		~TestEnemy() = default;

		bool Initialize(const PG_ENEMY_SPAWN& spawnDesc);
		void Update(float deltaTime);
		void Release();

		pixel_engine::PEISprite*	   GetBody     () const;
		pixel_engine::AnimSateMachine* GetAnimState() const;
		
		IAIController* GetController() const;

	private:
		bool InitializeBody(const PG_ENEMY_SPAWN& spawnDesc);
		bool InitializeAIController(const PG_ENEMY_SPAWN& spawnDesc);
		bool InitializeAppearance(const PG_ENEMY_SPAWN& spawnDesc);
		void UpdateAppearance(float deltaTime);

	private:
		std::unique_ptr<pixel_engine::QuadObject> m_pEnemyBody		{ nullptr };
		std::unique_ptr<pixel_engine::AnimSateMachine> m_pAnimState	{ nullptr };
		std::unique_ptr<ChaseAI> m_pAIController					{ nullptr };
	};
}
