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
#include "pixel_engine/render_manager/components/texture/texture.h"

namespace pixel_engine
{
	class PFE_API PNGLoader final: public ISingleton<PNGLoader>
	{
		friend class ISingleton<PNGLoader>;
	public:
		PNGLoader() = default;

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		std::unique_ptr<Texture> LoadTexture(_In_ const std::string& path);
	};
} // namespace pixel_engine
