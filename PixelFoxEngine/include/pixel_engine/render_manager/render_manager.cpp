#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"

_Use_decl_annotations_
pixel_engine::PERenderManager::PERenderManager(PEWindowsManager* windows)
    : m_pWindowsManager(windows), IFrameObject()
{
}

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

    //~ Initialize Render API
    m_handleStartEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    m_handleEndEvent   = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    CONSTRUCT_RENDER_API_DESC desc{};
    desc.StartEvent = m_handleStartEvent;
    desc.ExitEvent  = m_handleEndEvent;
    m_pRenderAPI    = std::make_unique<PERenderAPI>(&desc);

    INIT_RENDER_API_DESC renderDesc{};
    renderDesc.FullScreen    = m_pWindowsManager->IsFullScreen() ? TRUE: FALSE;
    renderDesc.Height        = m_pWindowsManager->GetWindowsHeight();
    renderDesc.Width         = m_pWindowsManager->GetWindowsWidth();
    renderDesc.WindowsHandle = m_pWindowsManager->GetWindowsHandle();
    
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

    SetEvent(m_handleStartEvent);

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

}

void pixel_engine::PERenderManager::OnFrameEnd()
{
    if (m_pRenderAPI)
    {
        m_pRenderAPI->WaitForPresent();
    }
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
