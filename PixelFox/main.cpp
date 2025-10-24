#include "application.h"
#include "pixel_engine/exceptions/base_exception.h"
#include "resource.h"

int CALLBACK WinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR         lpCmdLine,
    _In_ int           nCmdShow)
{
    try
    {
        pixel_engine::WINDOW_CREATE_DESC WindowsDesc{};
        WindowsDesc.Height      = 720;
        WindowsDesc.Width       = 1280u;
        WindowsDesc.IconId      = IDI_ICON1;
        WindowsDesc.WindowTitle = "PixelFoxTheGame";
        WindowsDesc.FullScreen  = false;

        pixel_engine::PIXEL_ENGINE_CONSTRUCT_DESC engineDesc{};
        engineDesc.WindowsDesc = &WindowsDesc;

        pixel_game::Application application{&engineDesc};

        if (not application.Init({})) return E_FAIL;

        return application.Execute({});
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
