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
#include "service_locator.h"

namespace pixel_engine
{
    std::shared_mutex ServiceLocator::s_mutex{};
    fox::vector<ServiceLocator::Deleter> ServiceLocator::s_deleters{};
    std::unordered_map<std::type_index, void*> ServiceLocator::s_services{};
} // namespace pixel_engine
