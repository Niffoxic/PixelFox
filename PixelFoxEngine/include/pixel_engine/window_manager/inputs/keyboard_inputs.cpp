#include "pch.h"
#include "keyboard_inputs.h"
#include <cstring>


pixel_engine::KeyboardInputs::KeyboardInputs() noexcept
{
    ClearAll();
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam) noexcept
{
    switch (message)
    {
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        const int key = static_cast<int>(wParam);
        if (!IsInside(key)) return false;

        if (!m_keyDown[key]) // key was pressed before 
        {
            if (!IsSetAutoRepeat(lParam))
                m_keyPressed[key] = true;
            m_keyDown[key] = true;
        }
        return true;
    }
    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        const int key = static_cast<int>(wParam);
        if (!IsInside(key)) return false;

        if (m_keyDown[key])
        {
            m_keyPressed[key] = true;
            m_keyDown[key] = false;
        }
        return true;
    }
    case WM_KILLFOCUS:
    case WM_SETFOCUS:
    {
        ClearAll();
        return false;
    }
    default:
        return false;
    }

	return false;
}

void pixel_engine::KeyboardInputs::OnFrameBegin() noexcept
{
}

void pixel_engine::KeyboardInputs::OnFrameEnd() noexcept
{
    std::memset(m_keyPressed,  0, sizeof(m_keyPressed ));
    std::memset(m_keyReleased, 0, sizeof(m_keyReleased));
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsKeyPressed(int virtualKey) const noexcept
{
	return IsInside(virtualKey) ? m_keyDown[virtualKey]: false;
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::WasKeyPressed(int virtualKey) const noexcept
{
    return IsInside(virtualKey) ? m_keyPressed[virtualKey] : false;
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::WasKeyReleased(int virtualKey) const noexcept
{
    return IsInside(virtualKey) ? m_keyReleased[virtualKey] : false;
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::WasChordPressed(int key, const pixel_engine::KeyboardMode& mode) const noexcept
{
    if (!WasKeyPressed(key)) return false;

    if ((mode & pixel_engine::KeyboardMode::Ctrl)   && !IsCtrlPressed())  return false;
    if ((mode & pixel_engine::KeyboardMode::Shift)  && !IsShiftPressed()) return false;
    if ((mode & pixel_engine::KeyboardMode::Alt)    && !IsAltPressed())   return false;
    if ((mode & pixel_engine::KeyboardMode::Super)  && !IsSuperPressed()) return false;

    return true;
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::WasMultipleKeyPressed(std::initializer_list<int> keys) const noexcept
{
    bool anyPressed = false;
    for (int key : keys) 
    {
        if (!IsKeyPressed(key)) return false;
        anyPressed = anyPressed || WasKeyPressed(key);
    }
    return anyPressed;
}

void pixel_engine::KeyboardInputs::ClearAll() noexcept
{
    std::memset(m_keyPressed,  0, sizeof(m_keyPressed));
    std::memset(m_keyDown,     0, sizeof(m_keyDown));
    std::memset(m_keyReleased, 0, sizeof(m_keyReleased));
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsSetAutoRepeat(LPARAM lParam) noexcept
{
	return (lParam & (1 << 30)) != 0;
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsCtrlPressed() const noexcept
{
    return m_keyDown[VK_CONTROL] || m_keyDown[VK_LCONTROL] || m_keyDown[VK_RCONTROL];
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsShiftPressed() const noexcept
{
	return m_keyDown[VK_SHIFT] || m_keyDown[VK_LSHIFT] || m_keyDown[VK_RSHIFT];
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsAltPressed() const noexcept
{
	return m_keyDown[VK_MENU] || m_keyDown[VK_LMENU] || m_keyDown[VK_RMENU];
}

_Use_decl_annotations_
bool pixel_engine::KeyboardInputs::IsSuperPressed() const noexcept
{
	return m_keyDown[VK_LWIN] || m_keyDown[VK_RWIN];
}
