// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "pch.h"
#include "windows_manager.h"

#include "pixel_engine/exceptions/win_exception.h"
#include "pixel_engine/core/event/event_queue.h"
#include "pixel_engine/core/event/event_windows.h"

#include "pixel_engine/utilities/logger/logger.h"

_Use_decl_annotations_
pixel_engine::PEWindowsManager::PEWindowsManager(const WINDOW_CREATE_DESC* desc)
{
    if (desc)
    {
        m_nWindowsHeight = desc->Height;
        m_nWindowsWidth  = desc->Width;
        m_szWindowTitle  = desc->WindowTitle;
        m_nIconID        = desc->IconId;
        m_bFullScreen    = desc->FullScreen;
    }
    else
    {
        logger::warning("No Description provided for Windows Manager creating default");
        m_nWindowsHeight = 720u;
        m_nWindowsWidth  = 1280u;
        m_szWindowTitle  = "Pixel Engine";
        m_nIconID        = 0u;
        m_bFullScreen    = false;
    }
}

pixel_engine::PEWindowsManager::~PEWindowsManager()
{
	if (not Release()) 
	{
        logger::error("Failed to Delete Windows Manager");
	}
}

_Use_decl_annotations_
bool pixel_engine::PEWindowsManager::ProcessMessage()
{
	MSG message{};

	//~ Process all pending messages for the frame
	while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
	{
		if (message.message == WM_QUIT) return true;

		TranslateMessage(&message);
		DispatchMessage (&message);
	}

	return false;
}

_Use_decl_annotations_
bool pixel_engine::PEWindowsManager::Initialize()
{
    if (!InitWindowScreen()) return false;
    if (auto handle = GetWindowsHandle())
        Mouse.AttachWindowHandle(handle);
	return true;
}

_Use_decl_annotations_
bool pixel_engine::PEWindowsManager::Release()
{
	return true;
}

_Use_decl_annotations_
void pixel_engine::PEWindowsManager::OnFrameBegin(float deltaTime)
{
	Keyboard.OnFrameBegin(deltaTime);
	Mouse   .OnFrameBegin(deltaTime);
}

void pixel_engine::PEWindowsManager::OnFrameEnd()
{
	Keyboard.OnFrameEnd();
	Mouse.OnFrameEnd();
}

_Use_decl_annotations_
HWND pixel_engine::PEWindowsManager::GetWindowsHandle() const
{
	return m_pWindowsHandle;
}

_Use_decl_annotations_
HINSTANCE pixel_engine::PEWindowsManager::GetWindowsInstance() const
{
	return m_pWindowsInstance;
}

_Use_decl_annotations_
void pixel_engine::PEWindowsManager::SetFullScreen(bool flag)
{
	if (flag == m_bFullScreen) return; // Same Request

	m_bFullScreen = flag;
	
	if (m_bFullScreen) TransitionToFullScreen();
	else TransitionToWindowedScreen();

    if (auto handle = GetWindowsHandle()) UpdateWindow(handle);

    RECT rt{};
    if (auto handle = GetWindowsHandle())
    {
        GetClientRect(handle, &rt);
    }
    else return;
    UINT width = rt.right - rt.left;
    UINT height = rt.bottom - rt.top;

    //~ Post Event to the queue
    if (m_bFullScreen) EventQueue::Post<FULL_SCREEN_EVENT>({ width, height });
    else EventQueue::Post<WINDOWED_SCREEN_EVENT>({ width, height });
}

_Use_decl_annotations_
float pixel_engine::PEWindowsManager::GetAspectRatio() const
{
	return static_cast<float>(m_nWindowsWidth) / static_cast<float>(m_nWindowsHeight);
}

_Use_decl_annotations_
void pixel_engine::PEWindowsManager::SetWindowTitle(const std::string& title)
{
	if (auto handle = GetWindowsHandle())
	{
        m_szWindowTitle = title;
		SetWindowText(handle, title.c_str());
	}
}

_Use_decl_annotations_
void pixel_engine::PEWindowsManager::SetWindowMessageOnTitle(const std::string& message) const
{
    if (auto handle = GetWindowsHandle())
    {
        std::string convert = m_szWindowTitle + " " + message;
        SetWindowText(handle, convert.c_str());
    }
}

_Use_decl_annotations_
bool pixel_engine::PEWindowsManager::InitWindowScreen()
{
	m_pWindowsInstance = GetModuleHandle(nullptr);

    WNDCLASSEX wc{};
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_OWNDC;
    wc.lpfnWndProc      = WindowProcSetup;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = sizeof(LONG_PTR);
    wc.hInstance        = m_pWindowsInstance;

    //~ Set Icon
    if (m_nIconID)
    {
        wc.hIcon   = LoadIcon(m_pWindowsInstance, MAKEINTRESOURCE(m_nIconID));
        wc.hIconSm = LoadIcon(m_pWindowsInstance, MAKEINTRESOURCE(m_nIconID));
    }
    else
    {
        wc.hIcon   = LoadIcon(nullptr, IDI_APPLICATION);
        wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    }
    wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = m_szWindowTitle.c_str();

    if (!RegisterClassEx(&wc))
    {
        THROW_WIN();
        return false;
    }

    DWORD style = WS_OVERLAPPEDWINDOW;

    RECT rect 
    { 0, 0,
      static_cast<LONG>(m_nWindowsWidth),
      static_cast<LONG>(m_nWindowsHeight) 
    };

    if (!AdjustWindowRect(&rect, style, FALSE))
    {
        THROW_WIN("Failed to adjust windows rect!");
        return false;
    }

    int adjustedWidth = rect.right - rect.left;
    int adjustedHeight = rect.bottom - rect.top;

    m_pWindowsHandle = CreateWindowEx(
        0,
        wc.lpszClassName,
        m_szWindowTitle.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        adjustedWidth, adjustedHeight,
        nullptr,
        nullptr,
        m_pWindowsInstance,
        this);

    if (!m_pWindowsHandle)
    {
        THROW_WIN();
        return false;
    }

    ShowWindow(m_pWindowsHandle, SW_SHOW);
    UpdateWindow(m_pWindowsHandle);

    return true;
}

_Use_decl_annotations_
LRESULT pixel_engine::PEWindowsManager::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (Keyboard.ProcessMessage(message, wParam, lParam)) return S_OK;
    if (Mouse.ProcessMessage   (message, wParam, lParam)) return S_OK;

    switch (message)
    {
    case WM_SIZE:
    {
        m_nWindowsWidth  = LOWORD(lParam);
        m_nWindowsHeight = HIWORD(lParam);
        EventQueue::Post<WINDOW_RESIZE_EVENT>({ m_nWindowsWidth, m_nWindowsHeight });
        return S_OK;
    }
    case WM_ENTERSIZEMOVE: // clicked mouse on title bar
    {
        EventQueue::Post<WINDOW_PAUSE_EVENT>({ true });
        return S_OK;
    }
    case WM_EXITSIZEMOVE: // not clicking anymore
    {
        EventQueue::Post<WINDOW_PAUSE_EVENT>({ false });
        return S_OK;
    }
    case WM_KILLFOCUS:
    {
        EventQueue::Post<WINDOW_PAUSE_EVENT>({ true });
        return S_OK;
    }
    case WM_SETFOCUS:
    {
        EventQueue::Post<WINDOW_PAUSE_EVENT>({ false });
        return S_OK;
    }
    case WM_CLOSE:
    {
        PostQuitMessage(0);
        return S_OK;
    }
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return S_OK;
}

_Use_decl_annotations_
LRESULT pixel_engine::PEWindowsManager::WindowProcSetup(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        CREATESTRUCT* create   = reinterpret_cast<CREATESTRUCT*>(lParam);
        PEWindowsManager* that = reinterpret_cast<PEWindowsManager*>(create->lpCreateParams);
        
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));
        SetWindowLongPtr(hwnd, GWLP_WNDPROC,  reinterpret_cast<LONG_PTR>(&WindowProcThunk));
        
        return that->MessageHandler(hwnd, message, wParam, lParam);
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

_Use_decl_annotations_
LRESULT pixel_engine::PEWindowsManager::WindowProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (auto that = reinterpret_cast<PEWindowsManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
    {
        return that->MessageHandler(hwnd, msg, wParam, lParam);
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void pixel_engine::PEWindowsManager::TransitionToFullScreen()
{
    if (not m_bFullScreen) return; //~ its windowed

    auto handle = GetWindowsHandle();
    if (!handle) return;
    GetWindowPlacement(handle, &m_WindowPlacement);

    SetWindowLong(handle, GWL_STYLE, WS_POPUP);
    SetWindowPos(
        handle,
        HWND_TOP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );
}

void pixel_engine::PEWindowsManager::TransitionToWindowedScreen() const
{
    if (m_bFullScreen) return; //~ its full screen

    auto handle = GetWindowsHandle();
    if (!handle) return;

    SetWindowLong(handle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(handle, &m_WindowPlacement);
    SetWindowPos
    (
        handle,
        nullptr,
        0, 0,
        0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );
}
