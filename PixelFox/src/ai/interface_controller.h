#pragma once

#include "pixel_engine/core/interface/interface_sprite.h"

namespace pixel_game
{
	class IAIController
	{
	public:
		IAIController()  = default;
		virtual ~IAIController() = default;

		virtual bool Init	(pixel_engine::PEISprite* aiBody) = 0;
		virtual void Update (float deltaTime)				  = 0;
		virtual bool Release()								  = 0;
		
		//~ killed by player
		virtual bool Kill		()				  = 0;
		virtual bool SetLifeSpan(float deltaTime) = 0;

		virtual void SetTarget(pixel_engine::PEISprite* target) = 0;
		virtual void SetActive(bool flag)						= 0;
		virtual bool IsActive () const							= 0;

		_NODISCARD virtual pixel_engine::PEISprite* GetBody()   const = 0;
		_NODISCARD virtual pixel_engine::PEISprite* GetTarget() const = 0;

		//~ helpers
		virtual float      DistanceFromPlayer() const = 0;
		virtual FVector2D  DirectionToTarget () const = 0;
		virtual bool       HasTarget		 () const = 0;

		virtual void OnTargetAcquired(_In_ pixel_engine::PEISprite* target) = 0;
		virtual void OnTargetLost() = 0;

	protected:
		virtual void UpdateAIDecision () = 0;
	};
}
