#pragma once
#include <type_traits>

namespace pixel_engine
{
	using TextureHandle = std::uint32_t;

	typedef struct _FOX_RGBA8
	{
		std::uint8_t r{}, g{}, b{}, a{ 255 };
	}RGBA8;

	typedef struct _FOX_RECT
	{
		// height and width can be -ve so normalize on use
		int x{}, y{}, w{}, h{};
	}FOX_RECT;

	// 2D affine (row-major)
	typedef struct _Mat3x2f
	{
		float m00{ 1 }, m01{ 0 }, m02{ 0 };
		float m10{ 0 }, m11{ 1 }, m12{ 0 };
	}Mat3x2f;

} // namespace pixel_engine
