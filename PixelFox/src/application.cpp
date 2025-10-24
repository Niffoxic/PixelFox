#include "application.h"

_Use_decl_annotations_
pixel_game::Application::Application(
	pixel_engine::PIXEL_ENGINE_CONSTRUCT_DESC const* desc)
	: pixel_engine::PixelEngine(desc)
{}

pixel_game::Application::~Application()
{

}

_Use_decl_annotations_
bool pixel_game::Application::InitApplication(pixel_engine::PIXEL_ENGINE_INIT_DESC const* desc)
{
	return true;
}

void pixel_game::Application::BeginPlay()
{
}

void pixel_game::Application::Tick(float deltaTime)
{
}

void pixel_game::Application::Release()
{
}

