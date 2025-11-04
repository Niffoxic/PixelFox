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

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/texture/texture.h"

#include "fox_math/vector.h"

namespace pixel_engine
{
	/// <summary>
	/// Takes Image Texture resource and creates another texture
	/// of size PixelUnit(32x32) * Scale(x, y) and 
	/// returns a sample of exact size
	/// </summary>
	class PFE_API BilinearSampler final: public ISingleton<BilinearSampler>
	{
		friend class ISingleton<BilinearSampler>;
	public:
		BilinearSampler() = default;

		// Given a tile size lets say 32x32 and scale it returns
		// the image scaled to the tile size of fast rendering
		// Example: tile=32, scale=(2,2) => 64x64 result.
		_NODISCARD _Check_return_ _Success_(return != nullptr)
		std::unique_ptr<Texture> GetSampledImage(
			_In_ const Texture*   rawImage,
			_In_ int              tileSize,
			_In_ const FVector2D& scale
		) const;

		//~ same but based on width and height instead of pixels
		_NODISCARD _Check_return_ _Success_(return != nullptr)
		std::unique_ptr<Texture> SampleToSize(
			_In_ const Texture* rawImage,
			_In_ uint32_t       outWidth,
			_In_ uint32_t       outHeight
		) const;

		//~ pixel a react description translate image
		_NODISCARD _Check_return_ _Success_(return != nullptr)
		std::unique_ptr<Texture> SampleRegionToSize(
			_In_ const Texture* rawImage,
			_In_ uint32_t           srcX,
			_In_ uint32_t           srcY,
			_In_ uint32_t           srcW,
			_In_ uint32_t           srcH,
			_In_ uint32_t           outWidth,
			_In_ uint32_t           outHeight
		) const;

		//~ Helper
		_NODISCARD _Check_return_
		int ClampIndex(
			_In_ int i,
			_In_ int n) const noexcept;

		_NODISCARD _Check_return_
		int MapYToMemory(
			_In_ int	  yLogical,
			_In_ uint32_t texHeight,
			_In_ Origin   origin) const noexcept;

		_NODISCARD _Check_return_
		uint8_t FetchChan(
			_In_ const uint8_t* base,
			_In_ uint32_t	    rowStride,
			_In_ uint32_t		bpp,
			_In_ int			x,
			_In_ int			y) const noexcept;

		_NODISCARD _Check_return_
		void FetchPixel(
			_In_ const uint8_t* base,
			_In_ uint32_t		rowStride,
			_In_ uint32_t		bpp,
			_In_ int			x,
			_In_ int			y,
			_In_ TextureFormat	fmt,
			_Out_ uint8_t&		r,
			_Out_ uint8_t&		g,
			_Out_ uint8_t&		b,
			_Out_ uint8_t&		a) const noexcept;

		_NODISCARD _Check_return_
		void StorePixel(
			_In_ uint8_t*		base,
			_In_ uint32_t		rowStride,
			_In_ uint32_t		bpp,
			_In_ int			x,
			_In_ int			y,
			_In_ TextureFormat	fmt,
			_In_ uint8_t		r,
			_In_ uint8_t		g,
			_In_ uint8_t		b,
			_In_ uint8_t		a) const noexcept;

		_NODISCARD _Check_return_
		uint8_t Lerp8(
			_In_ uint8_t a,
			_In_ uint8_t b,
			_In_ float	 t) const noexcept;
	};
} // namespace pixel_engine
