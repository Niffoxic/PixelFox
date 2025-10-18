#include "pixel_engine/window_manager/PEWindowsManager.h"

using namespace pixel_engine;

int CALLBACK WinMain(
    _In_ HINSTANCE     hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR         lpCmdLine,
    _In_ int           nCmdShow)
{
    PEWindowsManager windowsManager{};

    if (!windowsManager.OnInit())
    {
        MessageBox(nullptr, "Failed to initialize PEWindowsManager.", "PixelFox Error", MB_ICONERROR);
        return EXIT_FAILURE;
    }

    // Main message loop
    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PEWindowsManager::ProcessMessage()) return S_OK;
        windowsManager.OnLoopStart(0.0f);
        windowsManager.OnLoopEnd();
    }

    windowsManager.OnRelease();
    return S_OK;
}
