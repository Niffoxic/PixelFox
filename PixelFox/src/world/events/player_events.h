#pragma once

#include "fox_math/vector.h"

namespace pixel_game
{
	typedef struct _ON_PLAYER_HIT_EVENT
	{
		float	  damage;
		float	  hitKnockbackPower{ 0};
		FVector2D hitKnockbackDirection{ 0, 0 };
	} ON_PLAYER_HIT_EVENT;
} // pixel_game