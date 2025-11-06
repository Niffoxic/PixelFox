#include "application.h"

#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "pixel_engine/render_manager/components/texture/allocator/texture_resource.h"
#include "pixel_engine/render_manager/components/texture/allocator/tileset_allocator.h"

#include "pixel_engine/render_manager/components/font/font_allocator.h"
#include "pixel_engine/physics_manager/physics_queue.h"

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

	m_object = std::make_unique<pixel_engine::QuadObject>();
	m_object->SetPosition(-5, 0);
	m_object->SetScale(5, 5);
	m_object->SetTexture("assets/sprites/player/idle_left/left_0.png");

	pixel_engine::PhysicsQueue::Instance().AddObject(m_object.get());

	m_object_1 = std::make_unique<pixel_engine::QuadObject>();
	m_object_1->SetPosition(5, 0);
	m_object_1->SetScale(3, 3);
	m_object_1->SetTexture("assets/sprites/player/idle_left/left_0.png");

	pixel_engine::PhysicsQueue::Instance().AddObject(m_object_1.get());

	//~ init enemy
	PG_ENEMY_SPAWN desc{};
	desc.SpawnPoint = { 5, 5 };
	desc.Target = m_player.GetPlayerBody();
	m_enemy.Initialize(desc);

	pixel_engine::PhysicsQueue::Instance().AddObject(m_enemy.GetBody());
}

void pixel_game::Application::Tick(float deltaTime)
{    
	m_player.HandleInput(&m_pWindowsManager->Keyboard, deltaTime);
	
	m_player.Update(deltaTime);
	m_enemy.Update(deltaTime);
}

void pixel_game::Application::Release()
{
}
