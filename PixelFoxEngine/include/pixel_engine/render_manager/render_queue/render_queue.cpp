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
#include "render_queue.h"

#include "pixel_engine/render_manager/api/raster/raster.h"
#include "sampler/sample_allocator.h"
#include "pixel_engine/utilities/logger/logger.h"

#include <algorithm>

using namespace pixel_engine;

_Use_decl_annotations_
PERenderQueue::PERenderQueue(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc)
    : m_nScreenHeight(desc.ScreenHeight),
    m_nScreenWidth(desc.ScreenWidth),
    m_pCamera(desc.pCamera),
    m_nTilePx(desc.TilePx)
{
    m_nTileStep = 1.f / static_cast<float>(m_nTilePx);
    CreateCulling2D(desc);
    m_bDirtySprite.store(true, std::memory_order_release);
}

_Use_decl_annotations_
Camera2D* PERenderQueue::GetCamera() const { return m_pCamera; }

void PERenderQueue::Update()
{
    ApplyPendingFonts();
    if (m_bDirtyFont.exchange(false, std::memory_order_acq_rel))
        BuildFontsInOrder();

    if (m_bDirtySprite.exchange(false, std::memory_order_acq_rel))
    {
        BuildSpriteInOrder();
        m_bDirtySprite.store(false, std::memory_order_release);
    }
}

_Use_decl_annotations_
void PERenderQueue::Render(PERaster2D* pRaster)
{
    if (m_bDirtySprite.exchange(false, std::memory_order_acq_rel))
    {
        BuildSpriteInOrder();
        m_bDirtySprite.store(false, std::memory_order_release);
    }
        
    RenderSprite(pRaster);
    RenderFont(pRaster);
}

_Use_decl_annotations_
bool PERenderQueue::AddSprite(PEISprite* sprite)
{
    if (!sprite) return false;
    m_pendingAdd.push_back(sprite);
    m_bDirtySprite.store(true, std::memory_order_release);
    return true;
}

_Use_decl_annotations_
bool PERenderQueue::RemoveSprite(PEISprite* sprite)
{
    return sprite ? RemoveSprite(sprite->GetInstanceID()) : false;
}

_Use_decl_annotations_
bool PERenderQueue::RemoveSprite(UniqueId id)
{
    m_pendingRemove.push_back(id);
    m_bDirtySprite.store(true, std::memory_order_release);
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderQueue::AddFont(PEFont* font)
{
    if (!font) return false;
    m_pendingAddFont.push_back(font);
    m_bDirtyFont.store(true, std::memory_order_release);
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderQueue::RemoveFont(PEFont* font)
{
    return font ? RemoveFont(font->GetInstanceID()) : false;
}

_Use_decl_annotations_
bool pixel_engine::PERenderQueue::RemoveFont(UniqueId id)
{
    m_pendingRemoveFont.push_back(id);
    m_bDirtyFont.store(true, std::memory_order_release);
    return true;
}

_Use_decl_annotations_
void PERenderQueue::CreateCulling2D(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc)
{
    PFE_CULL2D_CONSTRUCT_DESC cullDesc{};
    cullDesc.Viewport = 
    {
        0, 0,
        static_cast<UINT>(desc.ScreenWidth),
        static_cast<UINT>(desc.ScreenHeight)
    };
    m_pCulling2D = std::make_unique<PECulling2D>(cullDesc);
}

_Use_decl_annotations_
void PERenderQueue::BuildDiscreteGrid(
    PEISprite*          sprite,
    Texture*            sampledTexture,
    int                 tilePx,
    PFE_SAMPLE_GRID_2D& out)
{
    const FVector2D center = sprite->GetPositionRelativeToCamera();
    const FVector2D axisU  = sprite->GetUAxisRelativeToCamera   ();
    const FVector2D axisV  = sprite->GetVAxisRelativeToCamera   ();
    int cols = 0, rows = 0;
    if (sampledTexture)
    {
        cols = static_cast<int>(sampledTexture->GetWidth());
        rows = static_cast<int>(sampledTexture->GetHeight());
    }
    else
    {
        const auto s = sprite->GetScale();
        cols = std::max(1, static_cast<int>(std::lround(s.x * tilePx)));
        rows = std::max(1, static_cast<int>(std::lround(s.y * tilePx)));
    }

    out.deltaAxisU = axisU / static_cast<float>(cols);
    out.deltaAxisV = axisV / static_cast<float>(rows);

    out.RowStart = center - axisU * 0.5f - axisV * 0.5f;

    out.cols = cols;
    out.rows = rows;
}

_Use_decl_annotations_
void PERenderQueue::RenderSprite(PERaster2D* pRaster)
{
    PFE_SAMPLE_GRID_2D grid{};
    for (auto* sprite : m_ppSortedSprites)
    {
        if (!sprite || !sprite->IsVisible()) continue;
        Texture*  sampled = sprite->GetSampledTexture();
        if (!sampled) continue;

        BuildDiscreteGrid(sprite, sampled, m_nTilePx, grid);

        grid.RowStart += FVector2D(m_nScreenWidth / 2, m_nScreenHeight / 2);

        if (m_pCulling2D->ShouldCullQuad(grid)) continue;

        PFE_CLIPPED_GRID cg;
        if (!ClipGridToViewport(grid, m_nScreenWidth, m_nScreenHeight, cg)) continue;

        PFE_RASTER_DRAW_CMD cmd
        {
            .startBase = grid.RowStart,
            .deltaAxisU = grid.deltaAxisU,
            .deltaAxisV = grid.deltaAxisV,
            .columnStartFrom = cg.columnStartFrom,
            .columneEndAt = cg.columneEndAt,
            .rowStartFrom = cg.j0,
            .rowEndAt = cg.j1,
            .totalColumns = grid.cols,
            .totalRows = grid.rows,
            .sampledTexture = sampled,
            .color = {100, 100, 100},
        };

        if (sprite->GetLayer() == ELayer::Background)
            pRaster->DrawQuadBackground(cmd);
        else
            pRaster->DrawQuadTile(cmd);
    }
}

void PERenderQueue::BuildSpriteInOrder()
{
    ApplyPending();

    if (!m_bDirtySprite.load(std::memory_order_acquire))
        return;

    fox::vector<PEISprite*> local;
    local.reserve(m_mapSprites.size());

    for (const auto& kv : m_mapSprites)
        if (kv.second) local.push_back(kv.second);

    std::stable_sort(local.begin(), local.end(),
        [](const PEISprite* a, const PEISprite* b)
        {
            const uint32_t la = static_cast<uint32_t>(a->GetLayer());
            const uint32_t lb = static_cast<uint32_t>(b->GetLayer());
            if (la != lb) return la < lb;
            return a->GetInstanceID() < b->GetInstanceID();
        });

    m_ppSortedSprites.swap(local);
    m_bDirtySprite.store(false, std::memory_order_release);
}

_Use_decl_annotations_
void pixel_engine::PERenderQueue::RenderFont(PERaster2D* pRaster)
{
    if (!pRaster || m_mapFonts.empty()) return;

    for (const auto& kv : m_mapFonts)
    {

        PEFont* font = kv.second;
        if (!font) continue;

        const auto& fontTextures = font->GetFontTextures();
        if (fontTextures.empty()) continue;

        for (const auto& glyph : fontTextures)
        {
            Texture* tex = glyph.sampledTexture;
            if (!tex) continue;

            FVector2D pos = glyph.startPosition;

            const int drawWidth  = static_cast<int>(tex->GetWidth());
            const int drawHeight = static_cast<int>(tex->GetHeight());

            PFE_RASTER_DRAW_CMD cmd
            {
                .startBase = pos,
                .deltaAxisU = FVector2D(1, 0),
                .deltaAxisV = FVector2D(0, 1),
                .columnStartFrom = 0,
                .columneEndAt = drawWidth,
                .rowStartFrom = 0,
                .rowEndAt = drawHeight,
                .totalColumns = drawWidth,
                .totalRows = drawHeight,
                .sampledTexture = tex,
                .color = {255, 255, 255},
            };
            pRaster->DrawQuadTile(cmd);
        }
    }
}

void pixel_engine::PERenderQueue::BuildFontsInOrder()
{
    fox::vector<PEFont*> local;
    local.reserve(m_mapFonts.size());

    for (const auto& kv : m_mapFonts)
        if (kv.second) local.push_back(kv.second);

    std::stable_sort(local.begin(), local.end(),
        [](const PEFont* a, const PEFont* b)
        {
            return a->GetInstanceID() < b->GetInstanceID();
        });

    m_ppFontsToRender.swap(local);
    m_bDirtyFont.store(false, std::memory_order_release);
}

_Use_decl_annotations_
float pixel_engine::PERenderQueue::Det2(
    float ax, float ay,
    float bx, float by) const noexcept
{
    return ax * by - ay * bx;
}

_Use_decl_annotations_
bool pixel_engine::PERenderQueue::InvertColumns(
    const FVector2D& deltaAxisU, const FVector2D& deltaAxisV,
    float& m00, float& m01,
    float& m10, float& m11) const noexcept
{
    const float det = Det2(deltaAxisU.x, deltaAxisU.y, deltaAxisV.x, deltaAxisV.y);
    
    if (std::abs(det) < 1e-8f) 
        return false;
    
    const float inv = 1.0f / det;

    // inverse 
    m00 = deltaAxisV.y  * inv;
    m01 = -deltaAxisV.x * inv;
    m10 = -deltaAxisU.y * inv;
    m11 = deltaAxisU.x  * inv;
    return true;
}

_Use_decl_annotations_
bool pixel_engine::PERenderQueue::ClipGridToViewport(
    const PFE_SAMPLE_GRID_2D& g,
    int vpW, int vpH,
    PFE_CLIPPED_GRID& out) const
{
    // Quad corners in screen space
    const FVector2D A = g.RowStart;
    const FVector2D B = { g.RowStart.x + g.deltaAxisU.x * g.cols, g.RowStart.y + g.deltaAxisU.y * g.cols };
    const FVector2D C = { g.RowStart.x + g.deltaAxisV.x * g.rows, g.RowStart.y + g.deltaAxisV.y * g.rows };
    const FVector2D D = { B.x + g.deltaAxisV.x * g.rows,        B.y + g.deltaAxisV.y * g.rows };

    // Quad AABB
    const float minX = std::min(std::min(A.x, B.x), std::min(C.x, D.x));
    const float maxX = std::max(std::max(A.x, B.x), std::max(C.x, D.x));
    const float minY = std::min(std::min(A.y, B.y), std::min(C.y, D.y));
    const float maxY = std::max(std::max(A.y, B.y), std::max(C.y, D.y));

    // Intersect with viewport
    const float clipMinX = std::max(0.0f, minX);
    const float clipMinY = std::max(0.0f, minY);
    const float clipMaxX = std::min(static_cast<float>(vpW), maxX);
    const float clipMaxY = std::min(static_cast<float>(vpH), maxY);

    if (clipMinX >= clipMaxX || clipMinY >= clipMaxY)
        return false; // fully off screen

    // Invert the 2x2 matrix to map
    float m00, m01, m10, m11;
    if (!InvertColumns(g.deltaAxisU, g.deltaAxisV, m00, m01, m10, m11)) return false;

    auto toIJ = [&](float px, float py) -> std::pair<float, float>
    {
        const float dx = px - g.RowStart.x;
        const float dy = py - g.RowStart.y;
            
        // transpose(i j) = inv * transpose(dx dy)
        const float i = m00 * dx + m01 * dy;
        const float j = m10 * dx + m11 * dy;
        return { i, j };
    };

    // Map the clipped rectangle corners into (i,j) space
    const auto [i00, j00] = toIJ(clipMinX, clipMinY);
    const auto [i10, j10] = toIJ(clipMaxX, clipMinY);
    const auto [i01, j01] = toIJ(clipMinX, clipMaxY);
    const auto [i11, j11] = toIJ(clipMaxX, clipMaxY);

    float iMin = std::min(std::min(i00, i10), std::min(i01, i11));
    float iMax = std::max(std::max(i00, i10), std::max(i01, i11));
    float jMin = std::min(std::min(j00, j10), std::min(j01, j11));
    float jMax = std::max(std::max(j00, j10), std::max(j01, j11));

    // Intersect with the valid grid index box
    int columnStartFrom = std::max(0, static_cast<int>(std::floor(iMin)));
    int columneEndAt = std::min(g.cols, static_cast<int>(std::ceil(iMax)));
    int j0 = std::max(0, static_cast<int>(std::floor(jMin)));
    int j1 = std::min(g.rows, static_cast<int>(std::ceil(jMax)));
    if (columnStartFrom >= columneEndAt || j0 >= j1) return false; // nothing visible

    // Build the clipped grid
    out.columnStartFrom = columnStartFrom;
    out.columneEndAt = columneEndAt;
    out.j0 = j0;
    out.j1 = j1;
    out.deltaAxisU = g.deltaAxisU;
    out.deltaAxisV = g.deltaAxisV;

    out.start = { g.RowStart.x + g.deltaAxisU.x * static_cast<float>(columnStartFrom) + g.deltaAxisV.x * static_cast<float>(j0),
                  g.RowStart.y + g.deltaAxisU.y * static_cast<float>(columnStartFrom) + g.deltaAxisV.y * static_cast<float>(j0) };
    return true;
}

void pixel_engine::PERenderQueue::ApplyPending()
{
    bool changed = false;

    if (!m_pendingRemove.empty())
    {
        for (const auto id : m_pendingRemove)
            changed |= (m_mapSprites.erase(id) != 0);
        m_pendingRemove.clear();
    }

    if (!m_pendingAdd.empty())
    {
        for (auto* s : m_pendingAdd)
        {
            if (!s) continue;
            const UniqueId id = s->GetInstanceID();
            if (!m_mapSprites.contains(id))
            {
                m_mapSprites[id] = s;
                changed = true;
            }
        }
        m_pendingAdd.clear();
    }

    if (changed)
        m_bDirtySprite.store(true, std::memory_order_release);
}

void pixel_engine::PERenderQueue::ApplyPendingFonts()
{
    bool changed = false;

    if (!m_pendingRemoveFont.empty())
    {
        for (const auto id : m_pendingRemoveFont)
            changed |= (m_mapFonts.erase(id) != 0);
        m_pendingRemoveFont.clear();
    }

    if (!m_pendingAddFont.empty())
    {
        for (auto* f : m_pendingAddFont)
        {
            if (!f) continue;
            const UniqueId id = f->GetInstanceID();
            if (!m_mapFonts.contains(id))
            {
                m_mapFonts[id] = f;
                changed = true;
            }
        }
        m_pendingAddFont.clear();
    }

    if (changed) m_bDirtyFont.store(true, std::memory_order_release);
}
