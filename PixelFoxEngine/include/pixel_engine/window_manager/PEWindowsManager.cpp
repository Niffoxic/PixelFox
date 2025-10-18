#include "pch.h"
#include "PEWindowsManager.h"

pixel_engine::PEWindowsManager::PEWindowsManager(const WINDOW_CREATE_DESC& desc)
{
	m_nWindowsHeight = desc.Height;
	m_nWindowsWidth  = desc.Width;
	m_szWindowTitle  = desc.WindowTitle;
}

pixel_engine::PEWindowsManager::~PEWindowsManager()
{
	if (not OnRelease()) 
	{
		// TODO: create a logger and record a failture in exiting.
	}
}

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

bool pixel_engine::PEWindowsManager::OnInit()
{
    if (!InitWindowScreen()) return false;
    Mouse.AttachWindowHandle(GetWindowsHandle());
	return true;
}

bool pixel_engine::PEWindowsManager::OnRelease()
{
	return true;
}

void pixel_engine::PEWindowsManager::OnLoopStart(float deltaTime)
{
	Keyboard.OnFrameBegin();
	Keyboard.OnFrameEnd();
}

void pixel_engine::PEWindowsManager::OnLoopEnd()
{
	Keyboard.OnFrameEnd();
	Mouse.OnFrameEnd();
}

HWND pixel_engine::PEWindowsManager::GetWindowsHandle() const
{
	return m_pWindowsHandle;
}

HINSTANCE pixel_engine::PEWindowsManager::GetWindowsInstance() const
{
	return m_pWindowsInstance;
}

void pixel_engine::PEWindowsManager::SetFullScreen(bool flag)
{
	if (flag == m_bFullScreen) return; // Same Request

	m_bFullScreen = flag;
	
	if (m_bFullScreen) TransitionToFullScreen();
	else TransitionToWindowedScreen();

	UpdateWindow(GetWindowsHandle());

	//~ TODO: Create EventManager and Send FullScreen and Windowed Screen Events;
}

float pixel_engine::PEWindowsManager::GetAspectRatio() const
{
	return static_cast<float>(m_nWindowsWidth) / static_cast<float>(m_nWindowsHeight);
}

void pixel_engine::PEWindowsManager::SetWindowTitle(const std::string& title) const
{
	if (auto handle = GetWindowsHandle())
	{
		SetWindowText(GetWindowsHandle(), title.c_str());
	}
}

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
    wc.hIcon            = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = nullptr;
    wc.lpszClassName    = m_szWindowTitle.c_str();
    wc.hIconSm          = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&wc))
    {
        // TODO: Create Window Exception Handler
        return false;
    }

    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    RECT rect 
    { 0, 0,
      static_cast<LONG>(m_nWindowsWidth),
      static_cast<LONG>(m_nWindowsHeight) 
    };

    if (!AdjustWindowRect(&rect, style, FALSE))
    {
        // TODO: Create Window Exception Handler
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
        // TODO: Create Window Exception Handler
        return false;
    }

    ShowWindow(m_pWindowsHandle, SW_SHOW);
    UpdateWindow(m_pWindowsHandle);

    return true;
}

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
        // TODO: Create Event Manager and Send Window Resize Event;
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

    GetWindowPlacement(GetWindowsHandle(), &m_WindowPlacement);

    SetWindowLong(GetWindowsHandle(), GWL_STYLE, WS_POPUP);
    SetWindowPos(
        GetWindowsHandle(),
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

    SetWindowLong(GetWindowsHandle(), GWL_STYLE, WS_OVERLAPPEDWINDOW);
    SetWindowPlacement(GetWindowsHandle(), &m_WindowPlacement);
    SetWindowPos
    (
        GetWindowsHandle(),
        nullptr,
        0, 0,
        0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW
    );
}
