#pragma once

#include <windows.h>
#include <thread>

inline void Win_SetThreadName(HANDLE h, const wchar_t* name) 
{
    using Fn = HRESULT(WINAPI*)(HANDLE, PCWSTR);
    static Fn p = (Fn)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),
        "PixelFoxWorkers");
    if (p) p(h, name);
}

// Priority helpers
inline void Win_SetThreadPriorityFixed(HANDLE h, int prio) 
{
    ::SetThreadPriority(h, prio);
    ::SetThreadPriorityBoost(h, TRUE);
}

// Affinity helpers single processor group machines
inline bool Win_SetThreadAffinityMask(HANDLE h, DWORD_PTR mask) 
{
    return ::SetThreadAffinityMask(h, mask) != 0;
}

// Reduce power throttling
inline void Win_DisablePowerThrottling(HANDLE h) 
{
    struct THREAD_POWER_THROTTLING_STATE
    {
        DWORD Version;
        DWORD ControlMask;
        DWORD StateMask;
    };
    constexpr DWORD ThreadPowerThrottling = 24;
    THREAD_POWER_THROTTLING_STATE s{};
    s.Version     = 1;
    s.ControlMask = 1;
    s.StateMask   = 0;
    ::SetThreadInformation(h, (THREAD_INFORMATION_CLASS)ThreadPowerThrottling, &s, sizeof(s));
}

inline void Win_BumpProcessPriority()
{
    ::SetPriorityClass(::GetCurrentProcess(), HIGH_PRIORITY_CLASS);
}

// One stop tuning
inline void Win_TuneStdThread(std::thread& t,
    int priority,
    int logicalCpu,
    const wchar_t* name) 
{
    HANDLE h = (HANDLE)t.native_handle();
    if (name) Win_SetThreadName(h, name);
    Win_SetThreadPriorityFixed(h, priority);
    Win_DisablePowerThrottling(h);
    if (logicalCpu >= 0) {
        const DWORD_PTR mask = (DWORD_PTR)1ull << logicalCpu;
        Win_SetThreadAffinityMask(h, mask);
    }
}
