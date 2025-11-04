// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once
#include "PixelFoxEngineAPI.h"

#include <atomic>
#include <mutex>
#include <utility>
#include <type_traits>
#include <cassert>

namespace pixel_engine
{
    /// <summary>
    /// Generic Singleton interface for all the services
    /// </summary>
    /// <typeparam name="T"> Service typename </typeparam>
    /// <typeparam name="LeakOnExit"> true: leak it on purpose </typeparam>
    template<class T, bool LeakOnExit = false>
    class ISingleton
    {
    public:
        ISingleton(_In_ const ISingleton&) = delete;
        ISingleton(_Inout_ ISingleton&&)   = delete;
        
        ISingleton& operator=(_In_ const ISingleton&) = delete;
        ISingleton& operator=(_Inout_ ISingleton&&)   = delete;

        // if needed to Initialize
        template<class... Args>
        _Ret_notnull_ static T& Init(_Inout_ Args&&... args)
        {
            if (auto* p = s_ptr.load(std::memory_order_acquire))
                return *p;

            std::call_once(s_once, [&] 
            {
                auto* created = new T(std::forward<Args>(args)...);
                s_ptr.store(created, std::memory_order_release);
            });

            return *s_ptr.load(std::memory_order_acquire);
        }

        //~ also creates it but if needed agrs then call init before instance
        _Ret_notnull_ _Success_(return != nullptr)
        static T& Instance()
        {
            if (auto* p = s_ptr.load(std::memory_order_acquire))
                return *p;

            std::call_once(s_once, [] 
            {
                auto* created = new T();
                s_ptr.store(created, std::memory_order_release);
            });

            return *s_ptr.load(std::memory_order_acquire);
        }

        //~ safe
        _Ret_maybenull_ static T* TryGet() noexcept
        {
            return s_ptr.load(std::memory_order_acquire);
        }

        _NODISCARD _Check_return_
        static bool IsInitialized() noexcept
        {
            return TryGet() != nullptr;
        }

        static void Destroy() noexcept
        {
            if _CONSTEXPR20 (!LeakOnExit) 
            {
                T* p = s_ptr.exchange(nullptr, std::memory_order_acq_rel);
                if (p) delete p;
            }
        }

    protected:
         ISingleton() = default;
        ~ISingleton() = default;

    private:
        inline static std::atomic<T*> s_ptr{ nullptr };
        inline static std::once_flag  s_once;
    };

} // namespace pixel_engine
