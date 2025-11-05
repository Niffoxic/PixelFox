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
#include "pixel_engine/render_manager/components/texture/resource/texture.h"

namespace pixel_engine
{
	typedef struct _PFE_CREATE_SAMPLE_TEXTURE
	{
		_In_ Texture*  texture;
		_In_ int       tileSize;
		_In_ FVector2D scaledBy;

		bool operator==(_In_ const _PFE_CREATE_SAMPLE_TEXTURE& other)
					   const noexcept
		{
			return	texture  == other.texture &&
					tileSize == other.tileSize &&
					scaledBy == other.scaledBy;
		}

		//~ since my fox::unorederd map dont support std::hash XD
		_NODISCARD _Check_return_
		std::string GetHashKey() const;

	} PFE_CREATE_SAMPLE_TEXTURE;

	/// <summary>
	/// Builds and caches sampled textures
	/// </summary>
	class PFE_API Sampler final : public ISingleton<Sampler>
	{
		friend class ISingleton<Sampler>;
	public:
		Sampler() = default;

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		Texture* BuildTexture(
			_Inout_ Texture*  unsampledTexture,
			_In_	FVector2D scale,
			_In_	int		  tilePx);

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		Texture* BuildTexture(_In_ const PFE_CREATE_SAMPLE_TEXTURE& desc);

	private:
		fox::unordered_map<std::string, std::unique_ptr<Texture>> m_sampledTextures{};
	};
} // pixel_engine
