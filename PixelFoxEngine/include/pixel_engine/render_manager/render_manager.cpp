#include "pch.h"
#include "render_manager.h"

#include "pixel_engine/core/event/event_windows.h"
#include "pixel_engine/utilities/logger/logger.h"

bool pixel_engine::PERenderManager::OnInit()
{
    SubscribeToEvents();
	return true;
}

bool pixel_engine::PERenderManager::OnRelease()
{
    UnSubscribeToEvents();
	return true;
}

void pixel_engine::PERenderManager::OnLoopStart(float deltaTime)
{
}

void pixel_engine::PERenderManager::OnLoopEnd()
{
}

void pixel_engine::PERenderManager::SubscribeToEvents()
{
    auto token = EventQueue::Subscribe<WINDOW_RESIZE_EVENT>([](const WINDOW_RESIZE_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Windows Resize Event");
            
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<FULL_SCREEN_EVENT>([](const FULL_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Full Screen Event");
    });
    m_eventTokens.push_back(token);

    token = EventQueue::Subscribe<WINDOWED_SCREEN_EVENT>([](const WINDOWED_SCREEN_EVENT& event)
    {
        logger::info(logger_config::LogCategory::Editor,
            "event: Recevied by RenderManager - Windowed Screen Event");
    });
    m_eventTokens.push_back(token);
}

void pixel_engine::PERenderManager::UnSubscribeToEvents()
{
    for (auto& token : m_eventTokens)
    {
        EventQueue::Unsubscribe(token);
    }
}
