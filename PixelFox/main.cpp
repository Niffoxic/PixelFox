#include "pixel_engine/window_manager/PEWindowsManager.h"
#include "pixel_engine/exceptions/base_exception.h"
#include "pixel_engine/core/event/event_queue.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/core/service/service_locator.h"

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
        LOGGER_CREATE_DESC cfg{};
        cfg.TerminalName          = "PixelFox Logger";
        cfg.EnableTerminal        = true;
        cfg.EnableAnsiTrueColor   = true;
        cfg.DuplicateToDebugger   = true;
        cfg.ShowTimestamps        = true;
        cfg.ShowThreadId          = true;
        cfg.ShowFileAndLine       = true;
        cfg.ShowFunction          = true;
        cfg.UseUtcTimestamps      = false;
        cfg.UseRelativeTimestamps = false;
        cfg.MinimumLevel          = logger_config::LogLevel::Trace;

        logger::init(cfg);

        logger::set_frame_index(1);

        logger::trace("trace boot {}", 1);
        logger::debug("dbg val = {}", 42);
        logger::info("engine online");
        logger::warning("low cache {}", 256);
        logger::success("hot-reload ready");

        using LC = logger_config::LogCategory;
        logger::info(LC::Render, "shader compiled: {}", "pbr_lit.hlsl");
        logger::debug(LC::Physics, "dt = {} ms", 16.6f);
        logger::warning(LC::Network, "rtt high: {} ms", 180);
        logger::error(LC::Physics, "working");

        logger::push_scope("Loading");
        logger::info("step 1");
        logger::push_scope("Textures");
        logger::info("A.dds");
        logger::pop_scope();
        logger::pop_scope();

        logger::set_use_relative_timestamps(true);
        logger::set_show_thread_id(true);
        logger::set_level(logger_config::LogLevel::Trace);

        logger::progress_begin(1, "Bake Lightmaps", 100);
        for (uint64_t i = 0; i < 100; ++i)
        {
            Sleep(10);
            logger::progress_update(1, i + 1, std::to_string(i) + " working");
        }
        logger::progress_end(1, true);

        std::thread worker([] 
        {
            logger::info(logger_config::LogCategory::System, "background job start");
            logger::push_scope("Job");
            logger::debug("doing things {}", 123);
            logger::pop_scope();
        });
        worker.join();

        WINDOW_CREATE_DESC desc{};
        desc.Height      = 500u;
        desc.Width       = 800u;
        desc.IconId      = IDI_ICON1;
        desc.WindowTitle = "PixelFox";

        PEWindowsManager windowsManager{ desc };

        if (!windowsManager.OnInit())
        {
            MessageBox(nullptr, "Failed to initialize PEWindowsManager.", "PixelFox Error", MB_ICONERROR);
            return EXIT_FAILURE;
        }

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

            windowsManager.OnLoopStart(0.0f);
            windowsManager.OnLoopEnd();

            EventQueue::DispatchAll();

            static uint64_t fi = 1;
            logger::set_frame_index(++fi);
        }

        windowsManager.OnRelease();
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
