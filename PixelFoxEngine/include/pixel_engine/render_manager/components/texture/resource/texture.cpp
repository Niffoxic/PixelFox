// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "texture.h"
#include <algorithm>


_Use_decl_annotations_
pixel_engine::Texture::Texture(
	const std::string&			 filepath,
	const uint32_t&				 width,
	const uint32_t&				 height,
	TextureFormat				 format,
	ColorSpace					 cs,
	fox::vector<unsigned char>&& data,
	const uint32_t				 strideBytes,
	Origin						 origin,
	bool						 premultipliedAlpha)
	: m_ppByteColors(std::move(data))
{
	m_szSourcePath		  = filepath;
	m_nWidth			  = width;
	m_nHeight			  = height;
	m_Format			  = format;
	m_ColorSpace		  = cs;
	m_Origin			  = origin;
	m_bPremultipliedAlpha = premultipliedAlpha;

	//~ prechecks
	const uint32_t minStride = 
		std::max<uint32_t>(1u, m_nWidth) * BytesPerPixel();
	m_nRowStride = (strideBytes != 0u) ? strideBytes : minStride;

	assert(m_nRowStride >= minStride);

	const size_t needed = static_cast<size_t>(m_nRowStride)
		* static_cast<size_t>(m_nHeight);

	assert(m_ppByteColors.size() >= needed);
}

_Use_decl_annotations_
std::string pixel_engine::Texture::GetFilePath() const noexcept
{
	return m_szSourcePath;
}

_Use_decl_annotations_
uint32_t pixel_engine::Texture::GetWidth() const noexcept
{
	return m_nWidth;
}

_Use_decl_annotations_
uint32_t pixel_engine::Texture::GetHeight() const noexcept
{
	return m_nHeight;
}

_Use_decl_annotations_
uint32_t pixel_engine::Texture::GetRowStride() const noexcept
{
	return m_nRowStride;
}

_Use_decl_annotations_
pixel_engine::TextureFormat pixel_engine::Texture::GetFormat() const noexcept
{
	return m_Format;
}

_Use_decl_annotations_
pixel_engine::ColorSpace pixel_engine::Texture::GetColorSpace() const noexcept
{
	return m_ColorSpace;
}

_Use_decl_annotations_
pixel_engine::Origin pixel_engine::Texture::GetOrigin() const noexcept
{
	return m_Origin;
}

_Use_decl_annotations_
bool pixel_engine::Texture::IsPremultipliedAlpha() const noexcept
{
	return m_bPremultipliedAlpha;
}

_Use_decl_annotations_
bool pixel_engine::Texture::HasAlpha() const noexcept
{
	return m_Format == TextureFormat::RGBA8;
}

_Use_decl_annotations_
bool pixel_engine::Texture::IsEmpty() const noexcept
{
	return (m_nWidth == 0u) || (m_nHeight == 0u) || m_ppByteColors.empty();
}

_Use_decl_annotations_
size_t pixel_engine::Texture::SizeInBytes() const noexcept
{
	return static_cast<size_t>(m_nRowStride) * static_cast<size_t>(m_nHeight);
}

_Use_decl_annotations_
pixel_engine::CONST_BYTE_VIEW pixel_engine::Texture::Data() const noexcept
{
	return { m_ppByteColors.data(), m_ppByteColors.size() };
}

_Use_decl_annotations_
pixel_engine::BYTE_VIEW pixel_engine::Texture::Data() noexcept
{
	return { m_ppByteColors.data(), m_ppByteColors.size() };
}

_Use_decl_annotations_
pixel_engine::CONST_BYTE_VIEW pixel_engine::Texture::Row(uint32_t y) const
{
	assert(y < m_nHeight);
	const size_t offset = static_cast<size_t>(y) * 
						  static_cast<size_t>(m_nRowStride);
	return { m_ppByteColors.data() + offset, m_nRowStride };
}

_Use_decl_annotations_
pixel_engine::BYTE_VIEW pixel_engine::Texture::Row(uint32_t y)
{
	assert(y < m_nHeight);
	const size_t offset = static_cast<size_t>(y) *
						  static_cast<size_t>(m_nRowStride);
	return { m_ppByteColors.data() + offset, m_nRowStride };
}

_Use_decl_annotations_
pixel_engine::PFE_FORMAT_R8G8B8_UINT pixel_engine::Texture::GetPixel(
	const uint32_t& x,
	const uint32_t& y) const
{
	assert(x < m_nWidth);
	assert(y < m_nHeight);

	const uint32_t bpp    = BytesPerPixel();
	const size_t   offset = static_cast<size_t>(y)			  *
							static_cast<size_t>(m_nRowStride) +
							static_cast<size_t>(x)			  *
							static_cast<size_t>(bpp);

	const uint8_t* p = m_ppByteColors.data() + offset;

	switch (m_Format)
	{
	case TextureFormat::R8:
		return PFE_FORMAT_R8G8B8_UINT{ p[0], p[0], p[0] };

	case TextureFormat::RG8:
		return PFE_FORMAT_R8G8B8_UINT{ p[0], p[1], 0u };

	case TextureFormat::RGB8:
		return PFE_FORMAT_R8G8B8_UINT{ p[0], p[1], p[2] };

	case TextureFormat::RGBA8:
		return PFE_FORMAT_R8G8B8_UINT{ p[0], p[1], p[2] };

	default:
		assert(false && "Unsupported format");
		return PFE_FORMAT_R8G8B8_UINT{ 0u, 0u, 0u };
	}
}

void pixel_engine::Texture::Release()
{
	m_ppByteColors.clear();
	m_ppByteColors.shrink_to_fit();
	m_nWidth = m_nHeight = m_nRowStride = 0u;
}

_Use_decl_annotations_
uint32_t pixel_engine::Texture::ChannelCount() const noexcept
{
	switch (m_Format)
	{
	case TextureFormat::R8:    return 1u;
	case TextureFormat::RG8:   return 2u;
	case TextureFormat::RGB8:  return 3u;
	case TextureFormat::RGBA8: return 4u;
	default:                   return 0u;
	}
}

_Use_decl_annotations_
uint32_t pixel_engine::Texture::BytesPerPixel() const noexcept
{
	return ChannelCount();
}
