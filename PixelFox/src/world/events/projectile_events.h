#pragma once
#include "pixel_engine/physics_manager/physics_api/collider/box_collider.h"

namespace pixel_game
{
	typedef struct _ON_PROJECTILE_HIT_EVENT
	{
		float damage;
		pixel_engine::BoxCollider* pCollider;
	} ON_PROJECTILE_HIT_EVENT;
} // pixel_game
