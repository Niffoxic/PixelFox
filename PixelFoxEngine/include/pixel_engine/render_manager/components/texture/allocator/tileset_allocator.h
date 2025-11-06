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
#include "fox_math/vector.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/texture/resource/tileset.h"

namespace pixel_engine
{
	typedef struct _PFE_CREATE_TILE_SET_FROM_SLICE
	{
		_In_ std::string tileFilePath;
		_In_ int         tileSize;
		_In_ FVector2D   scaledBy;
		_In_ int	     sliceWidth;
		_In_ int	     sliceHeight;
		_In_ int	     margin  = 0;
		_In_ int	     spacing = 0;

		bool operator==(_In_ const _PFE_CREATE_TILE_SET_FROM_SLICE& other)
			const noexcept
		{
			return	tileFilePath == other.tileFilePath &&
					tileSize	 == other.tileSize	   &&
					scaledBy	 == other.scaledBy	   &&
					sliceHeight  == other.sliceHeight  &&
					margin		 == other.margin	   &&
					spacing		 == other.spacing	   &&
					sliceWidth   == other.sliceWidth;
		}

		_NODISCARD _Check_return_
		std::string GetHashKey() const;

	} PFE_CREATE_TILE_SET_FROM_SLICE;

	/// <summary>
	/// Builds and caches tile set
	/// </summary>
	class PFE_API TileSetAllocator final : public ISingleton<TileSetAllocator>
	{
		friend class ISingleton<TileSetAllocator>;
	public:
		TileSetAllocator() = default;

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		TileSet* BuildTexture(_In_ const PFE_CREATE_TILE_SET_FROM_SLICE& desc);

	private:
		fox::unordered_map<std::string,
			std::unique_ptr<TileSet>> m_cachedTileSets{};
	};
} // pixel_engine
