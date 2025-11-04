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

#include "core/unordered_map.h"
#include "core/vector.h"

#include "pixel_engine/utilities/logger/logger.h"
#include "pixel_engine/core/interface/interface_singleton.h"

#include "texture.h"

namespace pixel_engine 
{
	/// <summary>
	/// Texture Resource Caches and returns a pointer to the resource
	/// </summary>
	class PFE_API TextureResource final: public ISingleton<TextureResource>
	{
		friend class ISingleton<TextureResource>;
	public:
		TextureResource() = default;

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		Texture* LoadTexture(_In_ const std::string& path);

	private:
		fox::unordered_map<std::string, std::unique_ptr<Texture>> m_cacheTextures{};
	};
} // pixel_engine
