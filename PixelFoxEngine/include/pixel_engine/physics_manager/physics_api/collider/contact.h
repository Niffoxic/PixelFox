#pragma once
#include "PixelFoxEngineAPI.h"

#include "box_collider.h"

namespace pixel_engine
{
	struct Contact
	{
		BoxCollider* A{ nullptr };
		BoxCollider* B{ nullptr };

		FVector2D Normal{ 0.f, 0.f };
		float     Penetration{ 0.f };
		FVector2D Point{ 0.f, 0.f };

		float Restitution{ 0.0f };
	};
} // namespace fox_physics
