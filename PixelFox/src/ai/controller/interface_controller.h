#pragma once
#include "pixel_engine/core/interface/interface_sprite.h"

namespace pixel_game
{
	class IAIController
	{
	public:
		virtual ~IAIController() = default;

		_NODISCARD _Check_return_
		virtual bool Init(_In_ pixel_engine::PEISprite* aiBody) = 0;

		virtual void Update(_In_ float deltaTime) = 0;

		_NODISCARD _Check_return_
		virtual bool Release() = 0;

		_NODISCARD _Check_return_
		virtual bool Kill() = 0;

		// if seconds = -1 it means infinite otherwise >= 0 if ok
		virtual void SetLifeSpan(_In_ float seconds) = 0;
		virtual void SetTarget(_In_opt_ pixel_engine::PEISprite* target)	    = 0;
		virtual void SetActive(_In_ bool flag)									= 0;

		_NODISCARD _Check_return_
		virtual bool IsActive() const = 0;

		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite* GetBody  () const = 0;

		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite* GetTarget() const = 0;

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

		_NODISCARD _Check_return_
		virtual bool HasTarget() const = 0;

		virtual void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) = 0;
		virtual void OnTargetLost() = 0;
	protected:
		virtual void UpdateAIDecision() = 0;
	};
}
