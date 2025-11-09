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
#include <type_traits>
#include <Windows.h>

#include "fox_math/vector.h"

namespace pixel_engine
{
	typedef struct _PFE_RECT
	{
		float x;
		float y;
		float w;
		float h;
	} PFE_RECT;

	typedef struct _PFE_FORMAT_R8_UINT
	{
		unsigned char Value;
	} PFE_FORMAT_R8_UINT;

	typedef struct _PFE_FORMAT_R8G8_UINT
	{
		PFE_FORMAT_R8_UINT R;
		PFE_FORMAT_R8_UINT G;
	} PFE_FORMAT_R8G8_UINT;

	typedef struct _PFE_FORMAT_R8G8B8_UINT
	{
		PFE_FORMAT_R8_UINT R;
		PFE_FORMAT_R8_UINT G;
		PFE_FORMAT_R8_UINT B;

		bool IsBlack() const
		{
			return R.Value <= 10 && G.Value <= 10 && B.Value <= 10;
		}

		bool IsWhite() const
		{
			return R.Value >= 250 && G.Value >= 250 && B.Value >= 250;
		}

	} PFE_FORMAT_R8G8B8_UINT;

	typedef struct _PE_IMAGE_BUFFER_DESC
	{
		UINT Height;
		UINT Width;
	} PE_IMAGE_BUFFER_DESC;

	typedef struct _PFE_VIEWPORT
	{
		UINT x;
		UINT y;
		UINT w;
		UINT h;

		bool operator==(const _PFE_VIEWPORT& other)
		{
			return	x == other.x && y == other.y &&
					w == other.w && h == other.h;
		}
	} PFE_VIEWPORT;

	typedef struct _PFE_AABB2D
	{
		float minX{ 0 }, minY{ 0 }, maxX{ 0 }, maxY{ 0 };
	} PFE_AABB2D;

	typedef struct _PFE_SAMPLE_GRID_2D
	{
		FVector2D RowStart;
		FVector2D deltaAxisU;
		FVector2D deltaAxisV;
		int cols{ 0 }, rows{ 0 };
	} PFE_SAMPLE_GRID_2D;

} // namespace pixel_engine
