#include "pch.h"
#include "tileset.h"
#include "pixel_engine/utilities/logger/logger.h"

#include "pixel_engine/render_manager/render_queue/sampler/sample_allocator.h"

pixel_engine::TileSet::TileSet(pixel_engine::Texture* source)
	: m_pSourceTexture(source)
{
}

bool pixel_engine::TileSet::Slice(const CREATE_TILE_SET_DESC& desc)
{
    if (!m_pSourceTexture)
    {
        logger::error("TextureSlicer: No source texture set!");
        return false;
    }

    const uint32_t texW = m_pSourceTexture->GetWidth();
    const uint32_t texH = m_pSourceTexture->GetHeight();

    if (desc.sliceWidth == 0 || desc.sliceHeight == 0 ||
        desc.sliceWidth > texW || desc.sliceHeight > texH)
    {
        logger::error("TextureSlicer: Invalid slice size ({}x{}) for texture ({}x{})",
            desc.sliceWidth, desc.sliceHeight, texW, texH);
        return false;
    }

    m_SliceW = desc.sliceWidth;
    m_SliceH = desc.sliceHeight;

    // Compute number of tiles per row/column
    m_Cols = (texW - 2 * desc.margin + desc.spacing) / (desc.sliceWidth + desc.spacing);
    m_Rows = (texH - 2 * desc.margin + desc.spacing) / (desc.sliceHeight + desc.spacing);

    if (m_Cols == 0 || m_Rows == 0)
    {
        logger::error("TextureSlicer: Computed 0 slices.");
        return false;
    }

    auto& srcData            = m_pSourceTexture->GetRaw();
    const uint32_t srcStride = m_pSourceTexture->GetRowStride();
    const uint32_t channels  =
        (m_pSourceTexture->GetFormat() == TextureFormat::RGBA8) ? 4 : 3;

    m_ppSlices.clear();
    m_ppSlices.reserve(static_cast<size_t>(m_Rows) * m_Cols);

    for (unsigned row = 0; row < m_Rows; ++row)
    {
        for (unsigned col = 0; col < m_Cols; ++col)
        {
            const unsigned srcX = desc.margin + col * (desc.sliceWidth + desc.spacing);
            const unsigned srcY = desc.margin + row * (desc.sliceHeight + desc.spacing);

            fox::vector<uint8_t> sliceData;
            sliceData.resize(static_cast<size_t>(desc.sliceWidth) * desc.sliceHeight * channels);

            for (unsigned y = 0; y < desc.sliceHeight; ++y)
            {
                const size_t srcOffset =
                    static_cast<size_t>(srcY + y) * srcStride +
                    static_cast<size_t>(srcX) * channels;

                const size_t dstOffset =
                    static_cast<size_t>(y) * desc.sliceWidth * channels;

                memcpy(sliceData.data() + dstOffset,
                    srcData.data() + srcOffset,
                    static_cast<size_t>(desc.sliceWidth) * channels);
            }

            auto slice = std::make_unique<Texture>(
                m_pSourceTexture->GetFilePath() + "_slice_" + std::to_string(row) + "_" + std::to_string(col),
                desc.sliceWidth,
                desc.sliceHeight,
                m_pSourceTexture->GetFormat(),
                m_pSourceTexture->GetColorSpace(),
                std::move(sliceData),
                desc.sliceWidth * channels,
                m_pSourceTexture->GetOrigin(),
                false);

            PFE_CREATE_SAMPLE_TEXTURE sampleDesc{};
            sampleDesc.scaledBy = desc.scale;
            sampleDesc.texture = slice.get();
            sampleDesc.tileSize = 32;
            auto sampledTexture = Sampler::Instance().BuildTexture(sampleDesc);

            if (sampledTexture)
            {
                m_ppSampledSlices.push_back(sampledTexture);
                m_ppSlices.emplace_back(std::move(slice));
            }
            else
            {
                logger::error("Failed to create tileset sample!");
                slice.reset();
                slice = nullptr;
            }
            
        }
    }

    logger::success("TextureSlicer: Created {}x{} = {} slices",
        m_Rows, m_Cols, m_ppSlices.size());
    return true;
}

pixel_engine::Texture* pixel_engine::TileSet::GetSlice(size_t index) const noexcept
{
    if (index >= m_ppSampledSlices.size()) return nullptr;
    return m_ppSampledSlices[index];
}

pixel_engine::Texture* pixel_engine::TileSet::GetSlice(
	unsigned row,
	unsigned col) const noexcept
{
    const size_t index = static_cast<size_t>(row) * m_Cols + col;
    return (index < m_ppSampledSlices.size()) ? m_ppSampledSlices[index]: nullptr;
}

void pixel_engine::TileSet::LogSlicesInfo() const
{
    logger::info("TextureSlicer: {}x{} slices ({} total), each {}x{}",
        m_Rows, m_Cols, m_ppSlices.size(), m_SliceW, m_SliceH);
}
