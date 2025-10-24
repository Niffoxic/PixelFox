#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"

// TODO: Upload to GPU
// via m_pDeviceContext->UpdateSubresource
// (m_pBackBuffer.Get(), 0, nullptr, cpuBytes, 0, 0);

_Use_decl_annotations_
pixel_engine::PERenderManager::PERenderManager(PEWindowsManager* windows)
    : m_pWindowsManager(windows), IFrameObject()
{
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::Initialize()
{
    logger::warning(pixel_engine::logger_config::LogCategory::Render,
        "Attempting to initialize RenderManager");

    SubscribeToEvents();

    //~ TODO: Initialize Render API
    m_pRenderAPI = std::make_unique<PERenderAPI>();

    INIT_RENDER_API_DESC renderDesc{};
    renderDesc.FullScreen    = m_pWindowsManager->IsFullScreen() ? TRUE: FALSE;
    renderDesc.Height        = m_pWindowsManager->GetWindowsHeight();
    renderDesc.Width         = m_pWindowsManager->GetWindowsWidth();
    renderDesc.WindowsHandle = m_pWindowsManager->GetWindowsHandle();

    logger::info("Creating with FullScreen?: {}", renderDesc.FullScreen);
    
    if (not m_pRenderAPI->Init(&renderDesc))

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "initialized RenderManager");

    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderManager::Release()
{
    UnSubscribeToEvents();
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
        m_pRenderAPI->Present();
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
