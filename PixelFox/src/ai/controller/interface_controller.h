#pragma once
#include "pixel_engine/core/interface/interface_sprite.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"

#include "world/state/character_state.h"
#include "fox_math/math.h"

#include <string>
#include <string_view>
#include <functional>
#include <sal.h>
#include <cfloat>
#include <cmath>

namespace pixel_game
{
	enum class EAttackDirection : uint8_t
	{
		Right		= 0,
		Up_Right	= 1,
		Up			= 2,
		Up_Left		= 3,
		Left		= 4,
		Down_Left	= 5,
		Down		= 6,
		Down_Right	= 7,
		Invalid		= 255
	};

	class IAIController;

	typedef struct _PE_AI_CONTROLLER_DESC
	{
		_In_ pixel_engine::PEISprite*				   pAiBody;
		_In_ pixel_engine::AnimSateMachine*			   pAnimStateMachine;
		_In_ std::function<void(class IAIController&, EAttackDirection)> fnOnAttackCallback;
		_In_ std::function<void(class IAIController&)> fnOnCantAttack;
	}PE_AI_CONTROLLER_DESC;

	class IAIController
	{
	public:
		using AttackCallbackType = std::function<void(IAIController&, EAttackDirection)>;
		using CantAttackCallbackType = std::function<void(class IAIController&)>;
	public:
		virtual ~IAIController() = default;

		//~ Life Time Managements
		_NODISCARD _Check_return_
		virtual bool Init(_In_ const PE_AI_CONTROLLER_DESC& desc) = 0;

		virtual void Update(_In_ float deltaTime) = 0;

		_NODISCARD _Check_return_
		virtual bool Release() = 0;

		_NODISCARD _Check_return_
		virtual bool Kill() = 0;

		// if seconds = -1 it means infinite otherwise >= 0 if ok
		virtual void SetLifeSpan(_In_ float seconds)					   = 0;
		virtual void SetTarget  (_In_opt_ pixel_engine::PEISprite* target) = 0;
		virtual void SetActive  (_In_ bool flag)						   = 0;

		_NODISCARD _Check_return_
		virtual bool IsActive() const = 0;

		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite* GetBody  () const = 0;

		//~ Target related stuff
		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite* GetTarget() const = 0;

		_NODISCARD _Check_return_
		virtual bool HasTarget() const = 0;

		virtual void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) = 0;
		virtual void OnTargetLost() = 0;

		//~ Attack Logic
		virtual void SetOnAttack(_In_opt_ AttackCallbackType cb)  = 0;
		virtual void FireAttack (_In_ EAttackDirection direction) = 0;

		virtual void SetOnCantAttack(_In_ CantAttackCallbackType cb) = 0;
		virtual void FireStopAttack() = 0;
		
		_NODISCARD virtual bool HasOnAttack() const = 0;

		virtual void SetAttackDistance(_In_ float d) = 0;
		_NODISCARD _Check_return_
		virtual float GetAttackDistance() const = 0;

		//~ helpers
		_NODISCARD _Check_return_
		float DistanceFromPlayer() const
		{
			auto* body   = GetBody();
			auto* target = GetTarget();

			if (!body || !target) return FLT_MAX;

			auto* rigidBodyA = body->GetRigidBody2D();
			auto* rigidBodyB = target->GetRigidBody2D();

			if (!rigidBodyA || !rigidBodyB) return FLT_MAX;

			const FVector2D a = rigidBodyA->GetPosition();
			const FVector2D b = rigidBodyB->GetPosition();

			return a.DistanceSq(a, b);
		}

		_NODISCARD _Check_return_
		FVector2D DirectionToTarget() const
		{
			auto* body = GetBody();
			auto* target = GetTarget();

			if (!body || !target) return { 0.f, 0.f };

			auto* rigidBodyA = body->GetRigidBody2D();
			auto* rigidBodyB = target->GetRigidBody2D();

			if (!rigidBodyA || !rigidBodyB) return { 0.f, 0.f };

			FVector2D a = rigidBodyA->GetPosition();
			FVector2D b = rigidBodyB->GetPosition();
			
			b -= a;
			return b.SafeNormalized();
		}

		__forceinline _NODISCARD _Check_return_
		int DirToOctant(_In_ const FVector2D& dir) const noexcept
		{
			float eps = 1e-6f; //~ fixes wierd bug where even tho it should be zero!
			if (std::fabs(dir.x) < eps && std::fabs(dir.y) < eps)
			{
				return -1;
			}
			
			float ang = std::atan2(-dir.y, dir.x);
			float pi  = fox_math::PI_v<float>;
			float oct = pi / 4.0f;
			
			if (ang < 0.0f) ang += 2.0f * pi;

			return static_cast<int>(std::floor((ang + oct * 0.5f) / oct)) & 7;
		}

		_NODISCARD _Check_return_
		CharacterState PickIdleFromDir(_In_ const FVector2D& dir) const noexcept
		{
			const int sector = DirToOctant(dir);
			if (sector < 0) return CharacterState::IDLE_RIGHT;

			switch (sector)
			{
			case 0:  return CharacterState::IDLE_RIGHT;
			case 1:  return CharacterState::IDLE_UP_RIGHT;
			case 2:  return CharacterState::IDLE_UP;
			case 3:  return CharacterState::IDLE_UP_LEFT;
			case 4:  return CharacterState::IDLE_LEFT;
			case 5:  return CharacterState::IDLE_DOWN_LEFT;
			case 6:  return CharacterState::IDLE_DOWN;
			case 7:  return CharacterState::IDLE_DOWN_RIGHT;
			default: return CharacterState::IDLE_RIGHT;
			}
		}

		_NODISCARD _Check_return_
		CharacterState PickWalkFromDir(_In_ const FVector2D& dir) const noexcept
		{
			const int sector = DirToOctant(dir);
			if (sector < 0) return CharacterState::WALK_RIGHT;

			switch (sector)
			{
			case 0:  return CharacterState::WALK_RIGHT;
			case 1:  return CharacterState::WALK_UP_RIGHT;
			case 2:  return CharacterState::WALK_UP;
			case 3:  return CharacterState::WALK_UP_LEFT;
			case 4:  return CharacterState::WALK_LEFT;
			case 5:  return CharacterState::WALK_DOWN_LEFT;
			case 6:  return CharacterState::WALK_DOWN;
			case 7:  return CharacterState::WALK_DOWN_RIGHT;
			default: return CharacterState::WALK_RIGHT;
			}
		}

		_NODISCARD _Check_return_
		CharacterState PickWalkOrIdleFromDir(_In_ const FVector2D& dir, _In_ bool moving) const noexcept
		{
			return moving ? PickWalkFromDir(dir) : PickIdleFromDir(dir);
		}
		
		_NODISCARD _Check_return_
		EAttackDirection DirToAttackDirection(_In_ const FVector2D& dir) const noexcept
		{
			const int sector = DirToOctant(dir);
			if (sector < 0) return EAttackDirection::Invalid;

			return static_cast<EAttackDirection>(sector);
		}

		_NODISCARD _Check_return_
		CharacterState PickAttackFromDir(_In_ const FVector2D& dir)
		{
			const EAttackDirection d = DirToAttackDirection(dir);
			switch (d)
			{
			case EAttackDirection::Right:      return CharacterState::ATTACK1_RIGHT;
			case EAttackDirection::Up_Right:   return CharacterState::ATTACK1_UP_RIGHT;
			case EAttackDirection::Up:		   return CharacterState::ATTACK1_UP;
			case EAttackDirection::Up_Left:    return CharacterState::ATTACK1_UP_LEFT;
			case EAttackDirection::Left:       return CharacterState::ATTACK1_LEFT;
			case EAttackDirection::Down_Left:  return CharacterState::ATTACK1_DOWN_LEFT;
			case EAttackDirection::Down:       return CharacterState::ATTACK1_DOWN;
			case EAttackDirection::Down_Right: return CharacterState::ATTACK1_DOWN_RIGHT;
			default:                           return CharacterState::ATTACK1_RIGHT;
			}
		}

	protected:
		virtual void UpdateAIDecision() = 0;
	};
}
