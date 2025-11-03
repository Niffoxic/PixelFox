#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/texture/texture.h"

#include "fox_math/vector.h"

namespace pixel_engine
{
	enum class EdgeMode : uint8_t { Clamp, Wrap, Mirror };

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
		_NODISCARD std::unique_ptr<Texture> GetSampledImage(
			_In_ const Texture*   rawImage,
			_In_ int              tileSize,
			_In_ const FVector2D& scale,
			_In_ EdgeMode         edge = EdgeMode::Clamp
		) const;

		//~ same but based on width and height instead of pixels
		_NODISCARD std::unique_ptr<Texture> SampleToSize(
			_In_ const Texture* rawImage,
			_In_ uint32_t       outWidth,
			_In_ uint32_t       outHeight,
			_In_ EdgeMode       edge = EdgeMode::Clamp
		) const;

		//~ pixel a react description translate image
		_NODISCARD std::unique_ptr<Texture> SampleRegionToSize(
			_In_ const Texture* rawImage,
			_In_ uint32_t           srcX,
			_In_ uint32_t           srcY,
			_In_ uint32_t           srcW,
			_In_ uint32_t           srcH,
			_In_ uint32_t           outWidth,
			_In_ uint32_t           outHeight,
			_In_ EdgeMode           edge = EdgeMode::Clamp
		) const;

	private:
		//~ Helper
		int ClampIndex(int i, int n) const noexcept;
		int WrapIndex(int i, int n) const noexcept;
		int MirrorIndex(int i, int n) const noexcept;
		int AddressIndex(int i, int n, EdgeMode mode) const noexcept;
		int MapYToMemory(int yLogical,
			uint32_t texHeight,
			Origin origin) const noexcept;

		uint8_t FetchChan(const uint8_t* base,
			uint32_t rowStride,
			uint32_t bpp,
			int x, int y) const noexcept;

		void FetchPixel(const uint8_t* base, uint32_t rowStride, uint32_t bpp,
			int x, int y, TextureFormat fmt,
			uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const noexcept;

		void StorePixel(uint8_t* base, uint32_t rowStride, uint32_t bpp,
			int x, int y, TextureFormat fmt,
			uint8_t r, uint8_t g, uint8_t b, uint8_t a) const noexcept;

		uint8_t Lerp8(uint8_t a, uint8_t b, float t) const noexcept;
	};
} // namespace pixel_engine
