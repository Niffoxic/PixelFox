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
#include "image.h"

#include "pixel_engine/utilities/logger/logger.h"

_Use_decl_annotations_
pixel_engine::PEImageBuffer::PEImageBuffer(const PE_IMAGE_BUFFER_DESC& desc)
{
	m_rowPitch  = (desc.Width * 3u + 3u) & ~3u;
	m_imageSize = m_rowPitch * desc.Height;
	m_height	= desc.Height;
	m_width		= desc.Width;

	m_imageData.resize(m_imageSize, 0u);
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_R(
	size_t atRow,
	size_t atColumn,
	const PFE_FORMAT_R8_UINT& r)
{
	if (not IsInside(atRow, atColumn)) 
	{
		logger::error("Attemping to write outside the image buffer");
		return false;
	}
	if (auto address = PixelAt(atRow, atColumn))
	{
		address[0] = r.Value;
		return true;
	}

	logger::error("attempted to write on a null pixel address Imagebuffer::WriteAt_R");
	return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_G(size_t atRow, size_t atColumn, const PFE_FORMAT_R8_UINT& g)
{
	if (!IsInside(atRow, atColumn))
	{
		logger::error("Attempting to write outside the image buffer");
		return false;
	}

	if (auto address = PixelAt(atRow, atColumn))
	{
		address[1] = g.Value;
		return true;
	}

	logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt_G");
	return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_B(size_t atRow, size_t atColumn, const PFE_FORMAT_R8_UINT& b)
{
	if (!IsInside(atRow, atColumn))
	{
		logger::error("Attempting to write outside the image buffer");
		return false;
	}
	if (auto address = PixelAt(atRow, atColumn))
	{
		address[2] = b.Value;
		return true;
	}

	logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt_B");
	return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_RG(size_t atRow, size_t atColumn, const PFE_FORMAT_R8G8_UINT& rg)
{
    if (!IsInside(atRow, atColumn))
    {
        logger::error("Attempting to write outside the image buffer");
        return false;
    }
    if (auto address = PixelAt(atRow, atColumn))
    {
        address[0] = rg.R.Value;
        address[1] = rg.G.Value;
        return true;
    }

    logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt_RG");
    return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_RB(size_t atRow, size_t atColumn, const PFE_FORMAT_R8G8_UINT& rb)
{
    if (!IsInside(atRow, atColumn))
    {
        logger::error("Attempting to write outside the image buffer");
        return false;
    }

    if (auto address = PixelAt(atRow, atColumn))
    {
        address[0] = rb.R.Value;
        address[2] = rb.G.Value;
        return true;
    }

    logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt_RB");
    return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt_GB(size_t atRow, size_t atColumn, const PFE_FORMAT_R8G8_UINT& gb)
{
    if (!IsInside(atRow, atColumn))
    {
        logger::error("Attempting to write outside the image buffer");
        return false;
    }

    if (auto address = PixelAt(atRow, atColumn))
    {
        address[1] = gb.R.Value;
        address[2] = gb.G.Value;
        return true;
    }

    logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt_GB");
    return false;
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::WriteAt(size_t atRow, size_t atColumn, const PFE_FORMAT_R8G8B8_UINT& rgb)
{
    if (!IsInside(atRow, atColumn))
    {
        //logger::error("Attempting to write outside the image buffer");
        return false;
    }

    if (auto address = PixelAt(atRow, atColumn))
    {
        address[0] = rgb.R.Value;
        address[1] = rgb.G.Value;
        address[2] = rgb.B.Value;
        return true;
    }

    logger::error("Attempted to write on a null pixel address Imagebuffer::WriteAt");
    return false;
}

_Use_decl_annotations_
void pixel_engine::PEImageBuffer::ClearImageBuffer(const PFE_FORMAT_R8G8B8_UINT& color)
{
    if (m_imageData.empty()) return;

    for (size_t row = 0; row < m_height; ++row)
    {
        unsigned char* rowPtr = m_imageData.data() + row * m_rowPitch;
        for (size_t col = 0; col < m_width; ++col)
        {
            if (unsigned char* px = rowPtr + col * 3u)
            {
                px[0] = color.R.Value;
                px[1] = color.G.Value;
                px[2] = color.B.Value;
            }
        }
    }
}

_Use_decl_annotations_
bool pixel_engine::PEImageBuffer::IsInside(size_t atRow, size_t atColumn) const noexcept
{
	return (atRow < m_height) && (atColumn < m_width);
}

_Use_decl_annotations_
unsigned char* pixel_engine::PEImageBuffer::PixelAt(size_t atRow, size_t atColumn) noexcept
{
	if (not IsInside(atRow, atColumn)) return nullptr;

	unsigned char* selectedRow = m_imageData.data() + m_rowPitch * atRow;
	unsigned char* address = selectedRow + atColumn * 3u;

	return address;
}
