// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "mouse_inputs.h"

#include <windowsx.h>

pixel_engine::PEMouseInputs::PEMouseInputs()
	:	m_nRawDeltaX(0), m_nRawDeltaY(0),
		m_nMouseWheelDelta(0), m_bCursorVisible(true),
        IInputHandler()
{
	ZeroMemory(m_bButtonDown, sizeof(m_bButtonDown));
	ZeroMemory(m_bButtonPressed, sizeof(m_bButtonPressed));
	m_pointPosition = { 0, 0 };
}

_Use_decl_annotations_
void pixel_engine::PEMouseInputs::AttachWindowHandle(HWND hWnd)
{
	if (hWnd)
	{
		RAWINPUTDEVICE device = { 0x01, 0x02, 0, hWnd };
		RegisterRawInputDevices(&device, 1u, sizeof(device));

		m_pWindowHandle = hWnd;
	}
}

_Use_decl_annotations_
std::string pixel_engine::PEMouseInputs::GetObjectName() const
{
    return "PixelEngineMouseInput";
}

_Use_decl_annotations_
bool pixel_engine::PEMouseInputs::Initialize()
{
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PEMouseInputs::Release()
{
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PEMouseInputs::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_MOUSEMOVE:
    {
        //~ extract mouse position 
        m_pointPosition.x = GET_X_LPARAM(lParam);
        m_pointPosition.y = GET_Y_LPARAM(lParam);
        return true;
    }
    case WM_LBUTTONDOWN:
    {
        //~ left mouse button clicked
        if (!m_bButtonDown[0])
        {
            m_bButtonPressed[0] = true;
            m_bButtonDown[0] = true;
        }
        return true;
    }
    case WM_LBUTTONUP:
    {
        //~ left mouse button click up
        m_bButtonDown[0] = false;
        return true;
    }
    case WM_RBUTTONDOWN:
    {
        //~ right button clicked
        if (!m_bButtonDown[1])
        {
            m_bButtonPressed[1] = true;
            m_bButtonDown[1]    = true;
        }
        return true;
    }
    case WM_RBUTTONUP:
    {
        //~ right button click up
        m_bButtonDown[1] = false;
        return true;
    }
    case WM_MBUTTONDOWN:
    {
        //~ wheeler button down
        if (!m_bButtonDown[2])
        {
            m_bButtonPressed[2] = true;
            m_bButtonDown[2]    = true;
        }
        return true;
    }
    case WM_MBUTTONUP:
    {
        //~ wheeler button click up
        m_bButtonDown[2] = false;
        return true;
    }
    case WM_MOUSEWHEEL:
    {
        //~ Wheeler in action
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        m_nMouseWheelDelta += delta;
        return true;
    }
    case WM_INPUT:
    {
        //~ thank you microsoft docuemntation
        UINT size = 0;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
        if (size > 0)
        {
            fox::vector<BYTE> data(size);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, data.data(), &size, sizeof(RAWINPUTHEADER)) == size)
            {
                RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(data.data());
                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    m_nRawDeltaX += raw->data.mouse.lLastX;
                    m_nRawDeltaY += raw->data.mouse.lLastY;
                }
            }
        }
        return true;
    }
    }
    return false;
}

_Use_decl_annotations_
void pixel_engine::PEMouseInputs::OnFrameBegin(float deltaTime) noexcept
{
}

void pixel_engine::PEMouseInputs::OnFrameEnd() noexcept
{
    m_nRawDeltaX        = 0;
    m_nRawDeltaY        = 0;
    m_nMouseWheelDelta  = 0;
    ZeroMemory(m_bButtonPressed, sizeof(m_bButtonPressed));
}

void pixel_engine::PEMouseInputs::HideCursor()
{
    if (m_bCursorVisible)
    {
        ShowCursor(FALSE);
        m_bCursorVisible = false;
    }
}

void pixel_engine::PEMouseInputs::UnHideCursor()
{
    if (!m_bCursorVisible)
    {
        ShowCursor(TRUE);
        m_bCursorVisible = true;
    }
}

void pixel_engine::PEMouseInputs::LockCursorToWindow() const
{
    RECT rect;
    if (GetClientRect(m_pWindowHandle, &rect))
    {
        POINT leftTop    { rect.left,  rect.top    };
        POINT rightBottom{ rect.right, rect.bottom };
        ClientToScreen(m_pWindowHandle, &leftTop);
        ClientToScreen(m_pWindowHandle, &rightBottom);
        
        rect = { leftTop.x, leftTop.y, rightBottom.x, rightBottom.y };
        ClipCursor(&rect);

        //~ force it to the center only
        int centerX = (leftTop.x + rightBottom.x) / 2;
        int centerY = (leftTop.y + rightBottom.y) / 2;
        SetCursorPos(centerX, centerY);
    }
}

void pixel_engine::PEMouseInputs::UnlockCursor() const
{
    ClipCursor(nullptr);
}
