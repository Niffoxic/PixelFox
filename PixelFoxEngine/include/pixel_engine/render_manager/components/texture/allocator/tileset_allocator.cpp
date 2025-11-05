#include "pch.h"
#include "tileset_allocator.h"
#include <sstream>

#include "texture_resource.h"

_Use_decl_annotations_
std::string pixel_engine::_PFE_CREATE_TILE_SET_FROM_SLICE::GetHashKey() const
{
    std::ostringstream oss;
    oss << tileFilePath
        << '(' << tileSize << ')'
        << '(' << scaledBy.x << ',' << scaledBy.y << ')'
        << '(' << sliceWidth << ',' << sliceHeight << ')'
        << '(' << margin << ',' << spacing;
    return oss.str();
}

_Use_decl_annotations_
pixel_engine::TileSet* pixel_engine::TileSetAllocator::
BuildTexture(const PFE_CREATE_TILE_SET_FROM_SLICE& desc)
{
    auto key = desc.GetHashKey();
    if (m_cachedTileSets.contains(key))
    {
        return m_cachedTileSets[key].get();
    }

    auto texture = TextureResource::Instance().LoadTexture(desc.tileFilePath);
    if (not texture)
    {
        logger::error("Failed to load {} as a whole", desc.tileFilePath);
        return nullptr;
    }
    auto tileSet = std::make_unique<TileSet>(texture);
    CREATE_TILE_SET_DESC tileDesc{};
    tileDesc.margin      = desc.margin;
    tileDesc.scale       = desc.scaledBy;
    tileDesc.sliceHeight = desc.sliceWidth;
    tileDesc.sliceWidth  = desc.sliceHeight;
    tileDesc.spacing     = desc.spacing;

    tileSet->Slice(tileDesc);

    if (tileSet->GetSliceCount() == 0) 
    {
        logger::error("Failed to slice {} tileset", desc.tileFilePath);
        return nullptr;
    }
    m_cachedTileSets[key] = std::move(tileSet);

    return m_cachedTileSets[key].get();
}
