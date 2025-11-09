#include "game_world.h"
#include "pixel_engine/utilities/logger/logger.h"

pixel_game::GameWorld::GameWorld(const PG_GAME_WORLD_CONSTRUCT_DESC& desc)
{
    m_pKeyboard = desc.pKeyboard;
    m_pWindows  = desc.pWindowsManager;

	BuildMainMenu(desc);
    BuildFPSFont();
    BuildLoadingDetails();

    m_pPlayer       = std::make_unique<PlayerCharacter>();

    m_pFiniteMap    = std::make_unique<FiniteMap>();
    m_pEnemySpawner = std::make_unique<EnemySpawner>(m_pPlayer.get());
}

pixel_game::GameWorld::~GameWorld()
{
    if (m_pFiniteMap) 
    {
        if (m_pFiniteMap->IsActive())
        {
            m_pFiniteMap->UnLoad();
        }
        m_pFiniteMap->Release();
    }
}

void pixel_game::GameWorld::Update(float deltaTime)
{
    ComputeFPS(deltaTime);
    KeyWatcher       (deltaTime);
	HandleTransition ();
	UpdateActiveState(deltaTime);
}

void pixel_game::GameWorld::BuildMainMenu(const PG_GAME_WORLD_CONSTRUCT_DESC& desc)
{
    PG_MAIN_MENU_DESC menuDesc{};
    menuDesc.pKeyboard = desc.pKeyboard;
    menuDesc.pWindows = desc.pWindowsManager;

    menuDesc.OnEnterPress_FiniteMap = [this]()
        {
            pixel_engine::logger::debug("MainMenu to Finite");
            SetState(GameState::Finite);
        };

    menuDesc.OnEnterPress_InfiniteMap = [this]()
        {
            pixel_engine::logger::debug("MainMenu to Infinite");
            SetState(GameState::Infinite);
        };

    menuDesc.OnEnterPress_Quit = []()
        {
            PostQuitMessage(0);
        };

    m_pMainMenu = std::make_unique<MainMenu>(menuDesc);
    m_pMainMenu->Init();
    m_pMainMenu->Show();
}

void pixel_game::GameWorld::AttachCameraToPlayer() const
{
}

void pixel_game::GameWorld::UpdateActiveState(float deltaTime)
{
    switch (m_eGameState)
    {
    case GameState::Menu:
    {
        if (m_pMainMenu) m_pMainMenu->Update(deltaTime);
        return;
    }
    case GameState::Finite:
    {
        if (m_pFiniteMap) m_pFiniteMap->Update(deltaTime);
        return;
    }
    case GameState::Infinite:
    {
        return;
    }
    }
}

void pixel_game::GameWorld::HandleTransition()
{
    if (m_ePrevGameState == m_eGameState) return;
    ExitState(m_ePrevGameState);
    EnterState(m_eGameState);
    m_ePrevGameState = m_eGameState;
}

void pixel_game::GameWorld::SetState(GameState next)
{
    if (next == m_eGameState) return;
    m_ePrevGameState = m_eGameState;
    m_eGameState     = next;
}

void pixel_game::GameWorld::EnterState(GameState state)
{
    switch (state)
    {
    case GameState::Menu:
    {
        if (m_pMainMenu) m_pMainMenu->Show();
        return;
    }
    case GameState::Finite: 
    {
        if (m_pMainMenu) m_pMainMenu->Hide();
        AttachCameraToPlayer();

        if (m_pFiniteMap)
        {
            if (!m_pFiniteMap->IsInitialized())
            {
                InitializeFiniteMap();
            }
            m_pFiniteMap->Start();
        }
        return;
    }
    case GameState::Infinite:
    {
        if (m_pMainMenu) m_pMainMenu->Hide();
        AttachCameraToPlayer();
        return;
    }
    }
}

void pixel_game::GameWorld::ExitState(GameState state)
{
    switch (state)
    {
    case GameState::Menu:
    {
        if (m_pMainMenu) m_pMainMenu->Hide();
        break;
    }
    case GameState::Finite:
    {
        if (m_pFiniteMap)
        {
            m_pFiniteMap->UnLoad();
        }
        return;
    }

    case GameState::Infinite:
    {
        return;
    }
    }
}

void pixel_game::GameWorld::KeyWatcher(float deltaTime)
{
    if (!m_pKeyboard) return;

    //~ Reduce cooldown timer
    if (m_nInputBlockTimer > 0.0f)
        m_nInputBlockTimer -= deltaTime;

    if (m_nInputBlockTimer <= 0.0f)
    {
        if (m_pKeyboard->WasKeyPressed(VK_ESCAPE))
        {
            switch (m_eGameState)
            {
            case GameState::Finite:
            {
                pixel_engine::logger::debug("Returning to Main Menu");
                if (m_pFiniteMap)
                {
                    m_pFiniteMap->UnLoad();
                }
                SetState(GameState::Menu);
                m_nInputBlockTimer = m_nInputDelay;
                return;
            }

            case GameState::Infinite:
            {
                pixel_engine::logger::debug("Returning to Main Menu");
                SetState(GameState::Menu);
                m_nInputBlockTimer = m_nInputDelay;
                return;
            }
            case GameState::Menu:
            default:
                break;
            }
        }
        if (m_pKeyboard->WasKeyPressed('F'))
        {
            ShowFPS();
            m_nInputBlockTimer = m_nInputDelay;
        }
    }
}

void pixel_game::GameWorld::BuildFPSFont()
{
    if (!m_fps)
    {
        m_fps = std::make_unique<pixel_engine::PEFont>();
        m_fps->SetPx(16);
        m_fps->SetPosition({ 10, 10 });
        m_fps->SetText("System Thread FPS: 0");

        pixel_engine::PERenderQueue::Instance().SetFPSPx(16);
    }
}

void pixel_game::GameWorld::ComputeFPS(float deltaTime)
{
    if (!m_bShowFPS) return;

    if (deltaTime < 0.0f) deltaTime = 0.0f;

    ++m_nFrameCount;
    m_nTimeElapsed += deltaTime;

    if (m_nTimeElapsed >= 1.0f)
    {
        m_nLastFps      = m_nFrameCount;
        m_nFrameCount   = 0;
        m_nTimeElapsed -= 1.0f;

        if (m_fps)
        {
            m_fps->SetText(std::format("System Thread FPS: {}", m_nLastFps));
        } 
    }
}

void pixel_game::GameWorld::ShowFPS()
{
    if (!m_fps) return;
    m_bShowFPS = !m_bShowFPS;

    auto& rq = pixel_engine::PERenderQueue::Instance();

    if (m_bShowFPS) // turning on
    {
        rq.EnableFPS(true, { 10, 40 });
        if (m_fps) rq.AddFont(m_fps.get());
        pixel_engine::logger::debug("FPS ON (last={})", m_nLastFps);
    }
    else // turning off
    {
        rq.EnableFPS(false, { 10, 40 });
        if (m_fps) rq.RemoveFont(m_fps.get());
        pixel_engine::logger::debug("FPS OFF");
    }
}

void pixel_game::GameWorld::BuildLoadingDetails()
{
    m_pLoadingInfo  = std::make_unique<pixel_engine::PEFont>();
    m_pLoadingTitle = std::make_unique<pixel_engine::PEFont>();
    m_pLoadingDesc  = std::make_unique<pixel_engine::PEFont>();

    m_pLoadingInfo ->SetPx(64);
    m_pLoadingTitle->SetPx(32);
    m_pLoadingDesc ->SetPx(16);

    float height = m_pWindows->GetWindowsHeight() / 2;
    float width  = m_pWindows->GetWindowsWidth() / 2;

    m_pLoadingInfo ->SetPosition({ 100, height - 200 });
    m_pLoadingTitle->SetPosition({ width - 350, height });
    m_pLoadingDesc ->SetPosition({ width - 300, height + 50 });
}

void pixel_game::GameWorld::InitializeFiniteMap()
{
    if (!m_pPlayer || !m_pEnemySpawner)
    {
        pixel_engine::logger::error("GameWorld::InitializeFiniteMap - Missing Player or EnemySpawner!");
        return;
    }

    //~ Set Loading Screen Active
    if (m_pLoadingInfo) 
    {
        m_pLoadingInfo->SetText("Loading Finite Map!");
        pixel_engine::PERenderQueue::Instance().AddFont(m_pLoadingInfo.get());
    }
    if (m_pLoadingTitle)
    {
        m_pLoadingTitle->SetText("Setting up Resouces");
        pixel_engine::PERenderQueue::Instance().AddFont(m_pLoadingTitle.get());
    }
    if (m_pLoadingDesc)
    {
        m_pLoadingDesc->SetText("fetching descriptions");
        pixel_engine::PERenderQueue::Instance().AddFont(m_pLoadingDesc.get());
    }

    LOAD_SCREEN_DETAILS details{};
    details.pLoadTitle       = m_pLoadingTitle.get();
    details.pLoadDescription = m_pLoadingDesc .get();

    // prepare map descriptor
    MAP_INIT_DESC mapDesc{};
    mapDesc.LoadScreen       = details;
    mapDesc.pPlayerCharacter = m_pPlayer.get();
    mapDesc.pEnemySpawner    = m_pEnemySpawner.get();
    mapDesc.MapDuration      = 120.f; 
    mapDesc.UseBounds        = true;
    mapDesc.Bounds           = { { -32.f, -32.f }, { 32, 32 } };
    mapDesc.Type             = EMapType::Finite;
    mapDesc.pKeyboard        = m_pKeyboard;
    mapDesc.OnMapComplete    = 
    [&]()
    {
        SetState(GameState::Menu);
    };

    mapDesc.OnMapComplete = [this]()
    {
        pixel_engine::logger::debug("Finite Map Complete → Returning to Menu");
        SetState(GameState::Menu);
    };

    // create and initialize
    m_pFiniteMap->Initialize(mapDesc);

    if (m_pLoadingInfo)
    {
        pixel_engine::PERenderQueue::Instance().RemoveFont(m_pLoadingInfo.get());
    }
    if (m_pLoadingTitle)
    {
        pixel_engine::PERenderQueue::Instance().RemoveFont(m_pLoadingTitle.get());
    }
    if (m_pLoadingDesc) 
    {
        pixel_engine::PERenderQueue::Instance().RemoveFont(m_pLoadingDesc.get());
    }

    if (!m_pLoadingDesc) pixel_engine::logger::error("FAILED TO RELESAE");

    m_pFiniteMap->Start();

    pixel_engine::logger::debug("GameWorld::InitializeFiniteMap - Finite Map initialized successfully!");
}
