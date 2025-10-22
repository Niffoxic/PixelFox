#include ""

#include "resource.h"

#include <string>
#include <thread>

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
        WINDOW_CREATE_DESC desc{};
        desc.Height      = 500u;
        desc.Width       = 800u;
        desc.IconId      = IDI_ICON1;
        desc.WindowTitle = "PixelFox";

        

        PEWindowsManager windowsManager{ desc };
        PERenderManager renderManager{};


        DependencyResolver resolver{};
        resolver.Register(&renderManager);
        resolver.Register(&windowsManager);

        resolver.AddDependency(&renderManager, &windowsManager);

        resolver.Init();

        auto subA = EventQueue::Subscribe<WindowResizedEvent>([](const WindowResizedEvent& event)
        {
            logger::info(logger_config::LogCategory::Editor, "event: {}", event.message);
            MessageBox(nullptr, event.message.c_str(), "Mesage", MB_OK);
        });

        EventQueue::Post(WindowResizedEvent{ "Is It woking?" });

        logger::info("fmt ints {} {} hex 0x{:X}", 7, 11, 255);
        logger::warning(logger_config::LogCategory::Asset, "cache {}% full", 78.5f);

        // loop
        while (true)
        {
            if (PEWindowsManager::ProcessMessage()) return S_OK;

            resolver.UpdateLoopStart(0.0f);
            resolver.UpdateLoopEnd();

            EventQueue::DispatchAll();

            static uint64_t fi = 1;
            logger::set_frame_index(++fi);
        }

        resolver.Shutdown();
        logger::close();
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
