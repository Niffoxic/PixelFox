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
#include <cstdint>
#include <string_view>

//~ Compile Time Hash FNV-1a
inline constexpr uint64_t PFE_API Hash64(std::string_view str) noexcept
{
    uint64_t hash = 1469598103934665603ull;
    for (char c : str)
        hash = (hash ^ static_cast<uint64_t>(c)) * 1099511628211ull;
    return hash;
}

//~ Unique Compile Time Hash Type
template<typename T>
constexpr uint64_t TypeHash() noexcept
{
#if defined(_MSC_VER)
    constexpr std::string_view sig = __FUNCSIG__;
    constexpr auto start = sig.find("TypeHash<") + 9;
    constexpr auto end = sig.find(">(void)");
    return Hash64(sig.substr(start, end - start));
#else
#error "Only Works on MSVC - Please Look at the readme.md"
#endif
}
