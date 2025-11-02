#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"

_Use_decl_annotations_
pixel_engine::PERenderManager::PERenderManager(
    PEWindowsManager* windows,
    GameClock* clock)
    : m_pWindowsManager(windows), m_pClock(clock), IFrameObject()
{}

pixel_engine::PERenderManager::~PERenderManager()
{
    if (not Release()) 
    {
        logger::error("Failed to cleanly delete RenderManager");
    }
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::Initialize()
{
    logger::warning(pixel_engine::logger_config::LogCategory::Render,
        "Attempting to initialize RenderManager");

    SubscribeToEvents();

    if (not InitializeCamera2D()) return false;
    if (not InitializeRenderAPI()) return false;
    
    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "initialized RenderManager");

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::Release()
{
    UnSubscribeToEvents();

    if (m_handleEndEvent) SetEvent(m_handleEndEvent);
    
    if (m_handleThread) 
    {
        WaitForSingleObject(m_handleThread, INFINITE);
        CloseHandle(m_handleThread);
        m_handleThread = nullptr;
    }

    SafeCloseEvent_(m_handleStartEvent);
    SafeCloseEvent_(m_handleEndEvent);

    m_pRenderAPI.reset();
	return true;
}

_Use_decl_annotations_
void pixel_engine::PERenderManager::OnFrameBegin(float deltaTime)
{
    HandleCameraInput(deltaTime);
    m_pCamera->OnFrameBegin(deltaTime);
}

void pixel_engine::PERenderManager::OnFrameEnd()
{
    m_pCamera->OnFrameEnd();

    if (m_pRenderAPI)
    {
        // TODO: Try something else of syncing
       // const auto t0 = std::chrono::high_resolution_clock::now();

        //m_pRenderAPI->WaitForPresent();

        //const auto t1 = std::chrono::high_resolution_clock::now();
        //const auto waitMs = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000.0;

        //logger::info(pixel_engine::logger_config::LogCategory::Render,
            //"WaitForPresent() blocked for {:.3f} ms", waitMs);
    }
}

bool pixel_engine::PERenderManager::InitializeCamera2D()
{
    m_pCamera = std::make_unique<Camera2D>();

    m_pCamera->SetViewportSize(
        (uint32_t)m_pWindowsManager->GetWindowsWidth(),
        (uint32_t)m_pWindowsManager->GetWindowsHeight());

    m_pCamera->SetViewportOrigin(
        { m_pWindowsManager->GetWindowsWidth() * 0.5f,
        m_pWindowsManager->GetWindowsHeight() * 0.5f });
    m_pCamera->SetScreenYDown(true);

    m_pCamera->SetZoom(1.f);

    m_pCamera->SetPosition({ 0.f, 0.f });
    m_pCamera->SetRotation(0.f);
    m_pCamera->SetScale({ 1.f, 1.f });

    m_pCamera->Initialize();

    return true;
}

bool pixel_engine::PERenderManager::InitializeRenderAPI()
{
    m_handleStartEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    m_handleEndEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    CONSTRUCT_RENDER_API_DESC desc{};
    desc.StartEvent = m_handleStartEvent;
    desc.ExitEvent = m_handleEndEvent;
    m_pRenderAPI = std::make_unique<PERenderAPI>(&desc);

    INIT_RENDER_API_DESC renderDesc{};
    renderDesc.FullScreen    = m_pWindowsManager->IsFullScreen() ? TRUE : FALSE;
    renderDesc.Height        = m_pWindowsManager->GetWindowsHeight();
    renderDesc.Width         = m_pWindowsManager->GetWindowsWidth();
    renderDesc.WindowsHandle = m_pWindowsManager->GetWindowsHandle();
    renderDesc.Clock         = m_pClock;
    renderDesc.Camera        = m_pCamera.get();

    if (not m_pRenderAPI->Init(&renderDesc)) return false;

    m_handleThread = CreateThread(
        nullptr,
        0,
        m_pRenderAPI->RenderThread,
        m_pRenderAPI.get(),
        0,
        nullptr
    );
    if (not m_handleThread) return false;

    //~ Boosts affinity and priority heheheheheh
    DWORD_PTR systemMask = 0, processMask = 0;
    if (GetProcessAffinityMask(GetCurrentProcess(), &processMask, &systemMask))
    {
        //~ Pin render api thread to the first available physical core
        DWORD_PTR renderCore = 1ull << (std::countr_zero(processMask));
        SetThreadAffinityMask(m_handleThread, renderCore);
    }

    //~ High priority for smoother frame pacing
    SetThreadPriority(m_handleThread, THREAD_PRIORITY_HIGHEST);
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetThreadDescription(m_handleThread, L"RenderAPIThread");

    if (m_handleStartEvent)
    {
        SetEvent(m_handleStartEvent);
    }
    else return false;

    return true;
}

void pixel_engine::PERenderManager::SubscribeToEvents()
{
    auto token = EventQueue::Subscribe<WINDOW_RESIZE_EVENT>([](const WINDOW_RESIZE_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
            "event: Recevied by RenderManager - Windows Resize Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<FULL_SCREEN_EVENT>([](const FULL_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
            "event: Recevied by RenderManager - Full Screen Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<WINDOWED_SCREEN_EVENT>([](const WINDOWED_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Render,
            "event: Recevied by RenderManager - Windowed Screen Event");
    });
    m_eventTokens.push_back(token);
}

void pixel_engine::PERenderManager::UnSubscribeToEvents()
{
    for (int i = 0; i < m_eventTokens.size(); i++)
    {
        EventQueue::Unsubscribe(m_eventTokens[i]);
    }
}

void pixel_engine::PERenderManager::SafeCloseEvent_(HANDLE& h)
{
    if (!h) return;

    DWORD flags = 0;
    if (!GetHandleInformation(h, &flags))
    {
        logger::error("Failed to Close: Invalid!!");
        h = nullptr;
        return;
    }

    if (!CloseHandle(h)) 
    {
        logger::error("Failed to Close Handle!");
    }
    h = nullptr;
}

void pixel_engine::PERenderManager::HandleCameraInput(float deltaTime)
{
    auto& kb = m_pWindowsManager->Keyboard;

    // === INPUT ===
    const float moveSpeed = 100.f, rotSpeed = 1.5f, zoomRate = 2.0f;
    const float minZoom = 0.1f, maxZoom = 2000.f;

    const float ang = m_pCamera->GetRotation();
    const float cc = std::cos(ang), sc = std::sin(ang);
    const FVector2D forward{ sc, -cc }; // +Y up in world
    const FVector2D right{ cc,  sc };

    FVector2D pan{ 0.f,0.f };
    if (kb.IsKeyPressed('W')) { pan.x += forward.x * moveSpeed * deltaTime; pan.y += forward.y * moveSpeed * deltaTime; }
    if (kb.IsKeyPressed('S')) { pan.x -= forward.x * moveSpeed * deltaTime; pan.y -= forward.y * moveSpeed * deltaTime; }
    if (kb.IsKeyPressed('A')) { pan.x -= right.x * moveSpeed * deltaTime; pan.y -= right.y * moveSpeed * deltaTime; }
    if (kb.IsKeyPressed('D')) { pan.x += right.x * moveSpeed * deltaTime; pan.y += right.y * moveSpeed * deltaTime; }
    if (pan.x || pan.y) m_pCamera->Pan(pan);

    if (kb.IsKeyPressed('Q')) m_pCamera->SetRotation(
        m_pCamera->GetRotation() - rotSpeed * deltaTime);
    if (kb.IsKeyPressed('E')) m_pCamera->SetRotation(m_pCamera->GetRotation() + rotSpeed * deltaTime);

    float z = m_pCamera->GetZoom(); bool zc = false;
    if (kb.IsKeyPressed(VK_UP)) { z *= std::exp(zoomRate * deltaTime); zc = true; }
    if (kb.IsKeyPressed(VK_DOWN)) { z *= std::exp(-zoomRate * deltaTime); zc = true; }
    if (zc) m_pCamera->SetZoom(std::clamp(z, minZoom, maxZoom));
}
