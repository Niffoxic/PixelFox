#include "game_world.h"

pixel_game::GameWorld::GameWorld(const PG_GAME_WORLD_CONSTRUCT_DESC& desc)
{
	PG_MAIN_MENU_DESC menuDesc{};
	menuDesc.pKeyboard = desc.pKeyboard;
	menuDesc.pWindows  = desc.pWindowsManager;
	m_pMainMenu = std::make_unique<MainMenu>(menuDesc);

	m_pMainMenu->Init();
	m_pMainMenu->Show();
}

pixel_game::GameWorld::~GameWorld()
{
}

void pixel_game::GameWorld::Update(float deltaTime)
{
	if (m_pMainMenu)
	{
		m_pMainMenu->Update(deltaTime);
	}
}

void pixel_game::GameWorld::AttachCameraToPlayer() const
{
}
