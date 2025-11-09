#pragma once

#include "ai/controller/interface_controller.h"
#include "fox_math/vector.h"
#include <filesystem>
#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"

namespace pixel_game
{
	typedef struct _PG_ENEMY_INIT_DESC
	{
		_In_ FVector2D				  SpawnPoint;
		_In_ FVector2D				  Scale;
		_In_ pixel_engine::PEISprite* pTarget;
	} PG_ENEMY_INIT_DESC;

	class IEnemy
	{
	public:
		virtual ~IEnemy() = default;

		virtual bool Initialize(_In_ const PG_ENEMY_INIT_DESC& desc) = 0;
		virtual void Update    (_In_ float deltaTime)			     = 0;
		virtual void Release   ()									 = 0;
		
		virtual void SetTypeName(const std::string& name) = 0;
		virtual std::string GetTypeName() const = 0;

		virtual bool						   IsActive	   () const = 0;
		virtual pixel_engine::PEISprite*       GetBody     () const = 0;
		virtual pixel_engine::AnimSateMachine* GetAnimState() const = 0;
		virtual pixel_engine::BoxCollider*     GetCollider () const = 0;
		
		virtual IAIController* GetController() const = 0;

		virtual void SetTarget(_In_opt_ pixel_engine::PEISprite* target) = 0;

		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite* GetTarget() const = 0;

		_NODISCARD _Check_return_
		virtual bool HasTarget() const = 0;

		//~ states
		_NODISCARD _Check_return_
		virtual bool IsDead() const = 0;

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		inline bool ValidatePathExists(const std::string& path) const
		{
			if (!std::filesystem::exists(path))
			{
				pixel_engine::logger::error("path dNT: {}", path);
				return false;
			}
			return true;
		}

		bool RangedEnemy() const { return m_bRangedEnemy; }

	protected:
		_NODISCARD _Check_return_
		virtual bool InitEnemyBody(const PG_ENEMY_INIT_DESC& desc) = 0;
		_NODISCARD _Check_return_
		virtual bool InitEnemyAnimStateMachine() = 0;
		_NODISCARD _Check_return_
		virtual bool InitEnemyAI(_In_ const PG_ENEMY_INIT_DESC& desc) = 0;

		//~ Configure Events
		virtual void SubscribeEvents		  () = 0;
		virtual void UnSubscribeEvents		  () = 0;
		virtual void InitCollisionCallback	  () = 0;
		virtual void AddColliderTags		  () = 0;
		
		//~ updates
		virtual void UpdateAnimState   (_In_ float deltaTime) = 0;
		virtual void UpdateAIController(_In_ float deltaTime) = 0;

	protected:
		bool m_bRangedEnemy{ false };
	};
} // namespace pixel_game
