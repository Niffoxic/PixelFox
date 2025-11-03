#pragma once

#include "PixelFoxEngineAPI.h"

#include "core/unordered_map.h"
#include "core/vector.h"

#include "pixel_engine/core/types.h"

#include <string>
#include <cstdint>

namespace pixel_engine
{
	typedef struct _BYTE_VIEW
	{
		uint8_t* data{ nullptr };
		size_t   size{ 0 };
		_NODISCARD bool empty() const noexcept { return size == 0; }
	} BYTE_VIEW;

	typedef struct _CONST_BYTE_VIEW
	{
		const uint8_t* data{ nullptr };
		size_t         size{ 0 };
		_NODISCARD bool empty() const noexcept { return size == 0; }
	} CONST_BYTE_VIEW;

	enum class TextureFormat : uint8_t
	{
		R8, RG8, RGB8, RGBA8
	};

	enum class ColorSpace : uint8_t
	{
		Linear, 
		sRGB
	};

	enum class Origin : uint8_t
	{
		TopLeft,
		BottomLeft
	};

	struct ImageView
	{
		uint32_t	   width;
		uint32_t	   height;
		uint32_t	   rowStride;
		TextureFormat  format;
		ColorSpace	   colorSpace;
		Origin		   origin;
		const uint8_t* ppBytes;

		_NODISCARD _Check_return_
		uint32_t BytesPerPixel() const noexcept
		{
			switch (format)
			{
			case TextureFormat::R8:    return 1u;
			case TextureFormat::RG8:   return 2u;
			case TextureFormat::RGB8:  return 3u;
			case TextureFormat::RGBA8: return 4u;
			default:                   return 0u;
			}
		}

		_NODISCARD _Check_return_
		CONST_BYTE_VIEW Row(uint32_t y) const
		{
			assert(ppBytes != nullptr);
			assert(y < height);
			const size_t offset = static_cast<size_t>(y) *
								  static_cast<size_t>(rowStride);
			return { ppBytes + offset, static_cast<size_t>(rowStride) };
		}
	};

	class PFE_API Texture
	{
	public:
		Texture(_In_ const std::string&			  filepath,
				_In_ const uint32_t&			  width,
				_In_ const uint32_t&			  height,
				_In_ TextureFormat				  format,
				_In_ ColorSpace					  cs,
				_In_ fox::vector<unsigned char>&& data,
				_In_ const uint32_t				  strideBytes		 = 0u,
				_In_ Origin						  origin			 = Origin::TopLeft,
				_In_ bool						  premultipliedAlpha = false
			);
		~Texture() = default;

		Texture(_In_ const Texture&) = delete;
		Texture(_In_ Texture&&)		 = delete;

		Texture& operator=(_In_ const Texture&)	= delete;
		Texture& operator=(_In_ Texture&&)		= delete;

		//~ Queries
		_NODISCARD _Check_return_ std::string	GetFilePath			() const noexcept;
		_NODISCARD _Check_return_ uint32_t		GetWidth			() const noexcept;
		_NODISCARD _Check_return_ uint32_t		GetHeight			() const noexcept;
		_NODISCARD _Check_return_ uint32_t		GetRowStride		() const noexcept;
		_NODISCARD _Check_return_ TextureFormat GetFormat			() const noexcept;
		_NODISCARD _Check_return_ ColorSpace	GetColorSpace		() const noexcept;
		_NODISCARD _Check_return_ Origin		GetOrigin			() const noexcept;
		_NODISCARD _Check_return_ bool			IsPremultipliedAlpha() const noexcept;
		_NODISCARD _Check_return_ bool			HasAlpha			() const noexcept;
		_NODISCARD _Check_return_ bool			IsEmpty				() const noexcept;
		_NODISCARD _Check_return_ size_t		SizeInBytes			() const noexcept;
		
		//~ Image Data
		_NODISCARD _Check_return_ CONST_BYTE_VIEW Data() const noexcept;
		_NODISCARD _Check_return_ BYTE_VIEW       Data()       noexcept;

		_NODISCARD _Check_return_ CONST_BYTE_VIEW Row(_In_ uint32_t y) const;
		_NODISCARD _Check_return_ BYTE_VIEW       Row(_In_ uint32_t y);

		//~ pixle data
		_NODISCARD _Check_return_ PFE_FORMAT_R8G8B8_UINT GetPixel(
			_In_ const uint32_t& x,
			_In_ const uint32_t& y) const;

		void Release();

		//~ Helpers
		_NODISCARD _Check_return_ uint32_t ChannelCount () const noexcept;
		_NODISCARD _Check_return_ uint32_t BytesPerPixel() const noexcept;

	private:
		uint32_t				   m_nWidth				{			0u			};
		uint32_t				   m_nHeight			{			0u			};
		uint32_t				   m_nRowStride			{			0u			};
		TextureFormat			   m_Format				{ TextureFormat::RGBA8  };
		ColorSpace				   m_ColorSpace			{	ColorSpace::Linear  };
		Origin					   m_Origin				{	Origin::TopLeft		};
		bool					   m_bPremultipliedAlpha{		false			};
		std::string				   m_szSourcePath		{		"Unknown"		};
		fox::vector<uint8_t>	   m_ppByteColors		{};
	};

} // namespace pixel_engine
