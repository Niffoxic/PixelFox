#pragma once

namespace pixel_game // buff events
{
	typedef struct _PLAYER_HP_BUFF_EVENT
	{
		float hp;
	}PLAYER_HP_BUFF_EVENT;

	typedef struct _PLAYER_SPEED_BUFF_EVENT
	{
		float speed;
	}PLAYER_SPEED_BUFF_EVENT;

	typedef struct _PLAYER_DAMANGE_BUFF_EVENT
	{
		float dmgBoost;
	} PLAYER_DAMANGE_BUFF_EVENT;

	typedef struct _PLAYER_LINEAR_ATTACK_SPEED_BUFF_EVENT
	{
		float speedBoost;
		float cdBoost{ 0.05 };
	}PLAYER_LINEAR_ATTACK_SPEED_BUFF_EVENT;

	typedef struct _PLAYER_SPECIAL_CD_DEC_EVENT
	{
		float decCDTime;
	}PLAYER_SPECIAL_CD_DEC_EVENT;
}