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

#include <string>
#include <sal.h>
#include "pixel_engine/utilities/id_allocator.h"

#include "interface_object.h"

namespace pixel_engine
{
	class PFE_API IFrameObject: public PEIObject
	{
	public:
		//~ Interface rules
		virtual void OnFrameBegin(_In_ float deltaTime) = 0;
		virtual void OnFrameEnd()						= 0;
	};
}
