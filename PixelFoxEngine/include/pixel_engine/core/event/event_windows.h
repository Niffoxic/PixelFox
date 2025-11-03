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
	bool BeingDrag;
} PFE_API WINDOW_PAUSE_EVENT;
