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
#include <Windows.h>

typedef struct _FULL_SCREEN_EVENT
{
	UINT Width;
	UINT Height;
} PFE_API FULL_SCREEN_EVENT;

typedef struct _WINDOWED_SCREEN_EVENT
{
	UINT Width;
	UINT Height;
} PFE_API WINDOWED_SCREEN_EVENT;

typedef struct _WINDOW_RESIZE_EVENT
{
	UINT Width;
	UINT Height;
} PFE_API WINDOW_RESIZE_EVENT;


typedef struct _WINDOW_DRAG_EVENT
{
	bool Paused;
} PFE_API WINDOW_PAUSE_EVENT;
