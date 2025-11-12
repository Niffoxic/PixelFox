#pragma once

#include "enemy/goblin/enemy_goblin.h"
#include "enemy/angry_goblin/enemy_angry_goblin.h"
#include "enemy/skeleton/enemy_skeleton.h"
#include "enemy/mushroom/enemy_mushroom.h"
#include "enemy/fire_dog/enemy_fire_dog.h"
#include "enemy/demon_lord/enemy_lord.h"
#include "enemy/registry_enemy.h"



using pixel_game::DemonLord;
REGISTER_BOSS_ENEMY(DemonLord);

using pixel_game::EnemyGoblin;
REGISTER_ENEMY(EnemyGoblin);

using pixel_game::EnemyAngryGoblin;
REGISTER_ENEMY(EnemyAngryGoblin);

using pixel_game::EnemySkeleton;
REGISTER_ENEMY(EnemySkeleton);

using pixel_game::EnemyMushroom;
REGISTER_ENEMY(EnemyMushroom);

using pixel_game::EnemyFireDog;
REGISTER_ENEMY(EnemyFireDog);
