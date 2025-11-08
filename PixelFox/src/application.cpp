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
	PG_GAME_WORLD_CONSTRUCT_DESC desc{};
	desc.pKeyboard       = &m_pWindowsManager->Keyboard;
	desc.pWindowsManager = m_pWindowsManager.get();
	m_pGameWorld = std::make_unique<GameWorld>(desc);
}

void pixel_game::Application::Tick(float deltaTime)
{    
	m_pGameWorld->Update(deltaTime);
}

void pixel_game::Application::Release()
{

}
