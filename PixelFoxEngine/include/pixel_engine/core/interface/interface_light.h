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

#include "pixel_engine/core/types.h"

namespace pixel_engine
{
	enum class LightBlendMode : std::uint8_t
	{
		Add = 0, // default for lights
		Alpha,
		Multiply
	};

	enum class LightFalloffModel : std::uint8_t
	{
		InverseSquare = 0,
		Exponential,
		Linear,
	};

	/// <summary>
	/// Sprite Light interface
	/// Manages Sprite as a light, effect nearby sprites with similar
	/// blending color (decays depending upon how far a sprite is from the 
	/// light source
	/// </summary>
	class __declspec(novtable) PFE_API ISpriteLight
	{
	public:
				 ISpriteLight() = default;
		virtual ~ISpriteLight() = default;

		//~ Child Must implement these interfaces

	protected:

	};
} // pixel_engine
