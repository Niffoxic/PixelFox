#pragma once

#include "fox_math/vector.h"

namespace pixel_game
{
	typedef struct _ENEMY_ATTACK_EVENT
	{
		float damage;
	} ENEMY_ATTACK_EVENT;

	typedef struct _EVENT_DIE_EVENT
	{
		FVector2D DeathLocation;
	}EVENT_DIE_EVENT;
} // pixel_game
