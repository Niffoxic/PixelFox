#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/render_manager/components/texture/allocator/tileset_allocator.h"

#include "pixel_engine/render_manager/components/font/font_allocator.h"
#include "pixel_engine/physics_manager/physics_queue.h"

#include "enemy/define_enemy.h"

_Use_decl_annotations_
pixel_game::Application::Application(
	pixel_engine::PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
	: pixel_engine::PixelEngine(desc)
{}

pixel_game::Application::~Application()
{}

_Use_decl_annotations_
bool pixel_game::Application::InitApplication(pixel_engine::PIXEL_ENGINE_INIT_DESC const* desc)
{
	return true;
}

void pixel_game::Application::BeginPlay()
{
	m_player.Initialize();
	
	m_pCamera2D = pixel_engine::PERenderQueue::Instance().GetCamera();
	m_pCamera2D->FollowSprite(m_player.GetPlayerBody());

	m_spawner = std::make_unique<EnemySpawner>(&m_player);

	pixel_game::PG_SPAWN_DESC spawnDesc{};
	spawnDesc.SpawnStartTime = 5.0f;
	spawnDesc.SpawnInterval = 3.0f;
	spawnDesc.SpawnMaxCount = 200;
	spawnDesc.SpawnRampTime = 120.0f;

	m_spawner->Initialize(spawnDesc);

	//~ test 1
	PG_ENEMY_INIT_DESC desc{};
	desc.pTarget = m_player.GetPlayerBody();
	desc.Scale = { 3, 3 };
	desc.SpawnPoint = { 20, 20 };
	m_goblin.Initialize(desc);
	pixel_engine::PhysicsQueue::Instance().AddObject(m_goblin.GetBody());

	//~ test 2
	m_enemy = RegistryEnemy::CreateEnemy("EnemyGoblin");
	desc.SpawnPoint = { -20, -20 };
	m_enemy->Initialize(desc);
	pixel_engine::PhysicsQueue::Instance().AddObject(m_enemy->GetBody());
}

void pixel_game::Application::Tick(float deltaTime)
{    
	m_player.HandleInput(&m_pWindowsManager->Keyboard, deltaTime);
	
	m_spawner->Update(deltaTime);
	m_player.Update(deltaTime);
	m_goblin.Update(deltaTime);
	m_enemy->Update(deltaTime);
}

void pixel_game::Application::Release()
{
	if (m_spawner)
	{
		m_spawner->Release();
		m_spawner.reset();
	}
}
