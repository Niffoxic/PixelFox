#include "pixel_engine/window_manager/PEWindowsManager.h"
#include "pixel_engine/exceptions/base_exception.h"
#include "pixel_engine/core/event/event_queue.h"

#include "pixel_engine/utilities/logger/logger.h"

#include "resource.h"

//~ Only Tests for now

using namespace pixel_engine;

struct WindowResizedEvent 
{
    std::string message;
};

int CALLBACK WinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR         lpCmdLine,
    _In_ int           nCmdShow)
{
    try
    {
        logger::Instance().test_log();

        WINDOW_CREATE_DESC desc{};
        desc.Height = 500u;
        desc.Width = 800u;
        desc.IconId = IDI_ICON1;
        desc.WindowTitle = "PixelFox";

        PEWindowsManager windowsManager{ desc };

        if (!windowsManager.OnInit())
        {
            MessageBox(nullptr, "Failed to initialize PEWindowsManager.", "PixelFox Error", MB_ICONERROR);
            return EXIT_FAILURE;
        }

        auto subA = EventQueue::Subscribe<WindowResizedEvent>([](const WindowResizedEvent& event)
        {
            MessageBox(nullptr, event.message.c_str(), "Mesage", MB_OK);
        });

        EventQueue::Post(WindowResizedEvent{ "Is It woking?" });

        while (true)
        {
            if (PEWindowsManager::ProcessMessage())
                return S_OK;

            windowsManager.OnLoopStart(0.0f);
            windowsManager.OnLoopEnd();

            EventQueue::DispatchAll();
        }

        windowsManager.OnRelease();
        return S_OK;
    }
    catch (const pixel_engine::BaseException& ex)
    {
        MessageBox(nullptr, ex.what(), "PixelFox Exception", MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }
    catch (const std::exception& ex)
    {
        MessageBox(nullptr, ex.what(), "Standard Exception", MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }
    catch (...)
    {
        MessageBox(nullptr, "Unknown fatal error occurred.", "PixelFox Crash", MB_ICONERROR | MB_OK);
        return EXIT_FAILURE;
    }
}
