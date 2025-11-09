#pragma once

#include "fox_math/vector.h"
#include "pixel_engine/render_manager/objects/quad/quad.h"
#include "pixel_engine/render_manager/components/animator/anim_state.h"

#include <sal.h>
#include <cfloat>
#include <functional>

namespace pixel_game
{
	class IProjectile;
	typedef struct _INIT_PROJECTILE_DESC 
	{
		std::function<void(_In_ IProjectile* projectile)> OnFire;
		std::function<void(_In_ IProjectile* projectile)> OnHit;
		std::function<void(_In_ IProjectile* projectile)> OnExpired;
		std::function<void(_In_ IProjectile* projectile)> OnActive;

		pixel_engine::PEISprite* pOwner{ nullptr };
	} INIT_PROJECTILE_DESC;

	class IProjectile
	{
	public:
		IProjectile() = default;
		virtual ~IProjectile() = default;

		_NODISCARD _Check_return_
		virtual bool Init(INIT_PROJECTILE_DESC& desc) = 0;

		virtual void Update(_In_ float deltaTime) = 0;

		_NODISCARD _Check_return_
		virtual bool Release() = 0;

		//~ Fire behavior
		_NODISCARD _Check_return_
		virtual bool Fire(
			_In_ const FVector2D& worldPos,
			_In_ const FVector2D& directionNorm,
			_In_ float speed = 0.0f) = 0;

		//~ State control
		virtual void Deactivate()			  = 0;
		virtual void SetActive (_In_ bool on) = 0;

		_NODISCARD _Check_return_
		virtual bool IsActive() const = 0;

		//~ getters
		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::PEISprite*  GetBody     () const = 0;

		_NODISCARD _Check_return_ _Ret_maybenull_
		virtual pixel_engine::BoxCollider* GetCollider() const = 0;

		//~ setters
		virtual void SetPosition(_In_ const FVector2D& pos) = 0;

		_NODISCARD _Check_return_
		virtual FVector2D GetPosition() const = 0;

		virtual void SetDirection(_In_ const FVector2D& dirNorm) = 0;

		_NODISCARD _Check_return_
		virtual FVector2D GetDirection() const = 0;

		virtual void SetSpeed(_In_ float unitsPerSecond) = 0;

		_NODISCARD _Check_return_
		virtual float GetSpeed() const = 0;

		//~ Lifetime
		_NODISCARD _Check_return_
		virtual bool SetLifeSpan(_In_ float seconds) = 0;

		_NODISCARD _Check_return_
		virtual float GetLifeSpan() const = 0;

		_NODISCARD _Check_return_
		virtual float GetTimeLeft() const = 0;

		//~ Damage
		virtual void SetDamage(_In_ float amount) = 0;

		_NODISCARD _Check_return_
		virtual float GetDamage() const = 0;

		//~ Collision
		virtual void OnHit() = 0;

		//~ animations
		virtual pixel_engine::AnimSateMachine* GetAnimStateMachine() const = 0;
	};
} // namespace pixel_game
