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
#include "sample_allocator.h"

#include "bilinear/bilinear_sampler.h"
#include "pixel_engine/utilities/logger/logger.h"

#include <sstream>

_Use_decl_annotations_
std::string pixel_engine::_PFE_CREATE_SAMPLE_TEXTURE::GetHashKey() const
{
    if (not texture) return "eos";
    std::ostringstream oss;
    oss << texture->GetFilePath()
        << '(' << tileSize << ')'
        << '(' << scaledBy.x << ',' << scaledBy.y << ')';

    return oss.str();
}

_Use_decl_annotations_
pixel_engine::Texture* pixel_engine::Sampler::BuildTexture(
    Texture*  unsampledTexture,
    FVector2D scale,
    int       tilePx)
{
    PFE_CREATE_SAMPLE_TEXTURE desc{};
    desc.scaledBy = scale;
    desc.tileSize = tilePx;
    desc.texture  = unsampledTexture;

    return BuildTexture(desc);
}

_Use_decl_annotations_
pixel_engine::Texture* pixel_engine::Sampler::BuildTexture(
    const PFE_CREATE_SAMPLE_TEXTURE& desc)
{
    auto hashKey = desc.GetHashKey();
    if (m_sampledTextures.contains(hashKey))
    {
        return m_sampledTextures[hashKey].get();
    }

    auto sampled = BilinearSampler::Instance().GetSampledImage(
        desc.texture, desc.tileSize, desc.scaledBy
    );

    if (not sampled)
    {
        logger::error("Failed to sample {}!",
            desc.texture->GetFilePath());
        return nullptr;
    }

    m_sampledTextures[hashKey] = std::move(sampled);
    return m_sampledTextures[hashKey].get();
}
