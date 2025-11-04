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

#include "event_queue.h"

using namespace pixel_engine;


namespace pixel_engine
{
    fox::unordered_map<std::type_index, EventQueue::TypeOps> EventQueue::s_mapRegistry{};

} // namespace pixel_engine
