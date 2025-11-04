#include "pch.h"
#include "render_queue.h"
#include "pixel_engine/render_manager/api/raster/raster.h"
#include <algorithm>

#include "pixel_engine/utilities/logger/logger.h"

#include <chrono>
using Clock = std::chrono::high_resolution_clock;
using namespace std::chrono_literals;

using namespace pixel_engine;

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

Camera2D* PERenderQueue::GetCamera() const { return m_pCamera; }

void PERenderQueue::Update()
{
    if (m_bDirtySprite.exchange(false, std::memory_order_acq_rel))
    {
        BuildSpriteInOrder();
        m_bDirtySprite.store(false, std::memory_order_release);
    }
}

void PERenderQueue::Render(PERaster2D* pRaster)
{
    if (m_bDirtySprite.exchange(false, std::memory_order_acq_rel))
    {
        BuildSpriteInOrder();
        m_bDirtySprite.store(false, std::memory_order_release);
    }
        
    RenderSprite(pRaster);
}

bool PERenderQueue::AddSprite(PEISprite* sprite)
{
    if (!sprite) return false;
    const UniqueId id = sprite->GetInstanceID();

    std::unique_lock lock(m_mutex);
    if (m_mapSprites.contains(sprite->GetInstanceID())) return false;

    m_mapSprites[sprite->GetInstanceID()] = sprite;

    m_bDirtySprite.store(true, std::memory_order_release);
    return true;
}

bool PERenderQueue::RemoveSprite(PEISprite* sprite)
{
    return sprite ? RemoveSprite(sprite->GetInstanceID()) : false;
}

bool PERenderQueue::RemoveSprite(UniqueId id)
{
    std::unique_lock lock(m_mutex);
    const auto erased = m_mapSprites.erase(id);
    if (erased) m_bDirtySprite.store(true, std::memory_order_release);
    return erased != 0;
}

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

void PERenderQueue::BuildDiscreteGrid(
    PEISprite* sprite,
    int tilePx,
    PFE_SAMPLE_GRID_2D& out)
{
    const FVector2D center = sprite->GetPositionRelativeToCamera();
    const FVector2D axisU  = sprite->GetUAxisRelativeToCamera();
    const FVector2D axisV  = sprite->GetVAxisRelativeToCamera();

    int cols = 0, rows = 0;
    if (auto* tex = sprite->GetTexture())
    {
        cols = static_cast<int>(tex->GetWidth());
        rows = static_cast<int>(tex->GetHeight());
    }
    else
    {
        const auto s = sprite->GetScale();
        cols = std::max(1, static_cast<int>(std::lround(s.x * tilePx)));
        rows = std::max(1, static_cast<int>(std::lround(s.y * tilePx)));
    }

    out.dU = axisU / static_cast<float>(cols);
    out.dV = axisV / static_cast<float>(rows);

    out.RowStart = center - axisU * 0.5f - axisV * 0.5f;

    out.cols = cols;
    out.rows = rows;
}

void PERenderQueue::RenderSprite(PERaster2D* pRaster)
{
    PFE_SAMPLE_GRID_2D grid{};
    for (auto* sprite : m_ppSortedSprites)
    {
        if (!sprite || !sprite->IsVisible()) continue;

        BuildDiscreteGrid(sprite, m_nTilePx, grid);

        // pixel coordinate space
        grid.RowStart += FVector2D(m_nScreenWidth / 2, m_nScreenHeight / 2);

        if (m_pCulling2D->ShouldCullQuad(grid)) continue;

        PFE_CLIPPED_GRID cg;
        if (!ClipGridToViewport(grid, m_nScreenWidth, m_nScreenHeight, cg)) continue;

        if (auto* texture = sprite->GetTexture())
        {
            if (sprite->GetLayer() == 0)
            {
                pRaster->DrawQuadClippedMT(grid.RowStart,
                    grid.dU, grid.dV, cg.i0, cg.i1,
                    cg.j0, cg.j1, grid.cols, grid.rows, texture);
            }
            else
            {
                pRaster->DrawDiscreteQuad(
                    grid.RowStart,
                    grid.dU, grid.dV,
                    grid.cols, grid.rows,
                    texture
                );
            }
        }
    }
}

void PERenderQueue::BuildSpriteInOrder()
{
    fox::vector<PEISprite*> local;
    local.reserve(m_mapSprites.size());

    for (const auto& kv : m_mapSprites)
        if (kv.second)
            local.push_back(kv.second);

    std::stable_sort(local.begin(), local.end(),
    [](const PEISprite* a, const PEISprite* b)
    {
        const uint32_t la = a->GetLayer(), lb = b->GetLayer();
        if (la != lb) return la < lb;
        return a->GetInstanceID() < b->GetInstanceID();
    });

    m_ppSortedSprites.swap(local);
}

float pixel_engine::PERenderQueue::Det2(float ax, float ay, float bx, float by) const noexcept
{
    return ax * by - ay * bx;
}

bool pixel_engine::PERenderQueue::InvertColumns(const FVector2D& dU, const FVector2D& dV,
    float& m00, float& m01, float& m10, float& m11) const noexcept
{
    const float det = Det2(dU.x, dU.y, dV.x, dV.y);
    
    if (std::abs(det) < 1e-8f) 
        return false;
    
    const float inv = 1.0f / det;

    // inverse 
    m00 = dV.y * inv;  m01 = -dV.x * inv;
    m10 = -dU.y * inv;  m11 = dU.x * inv;
    return true;
}

bool pixel_engine::PERenderQueue::ClipGridToViewport(const PFE_SAMPLE_GRID_2D& g, int vpW, int vpH, PFE_CLIPPED_GRID& out) const
{
    // Quad corners in screen space
    const FVector2D A = g.RowStart;
    const FVector2D B = { g.RowStart.x + g.dU.x * g.cols, g.RowStart.y + g.dU.y * g.cols };
    const FVector2D C = { g.RowStart.x + g.dV.x * g.rows, g.RowStart.y + g.dV.y * g.rows };
    const FVector2D D = { B.x + g.dV.x * g.rows,        B.y + g.dV.y * g.rows };

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

    // Invert the 2x2 matrix to map P
    float m00, m01, m10, m11;
    if (!InvertColumns(g.dU, g.dV, m00, m01, m10, m11)) return false;

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
    int i0 = std::max(0, static_cast<int>(std::floor(iMin)));
    int i1 = std::min(g.cols, static_cast<int>(std::ceil(iMax)));
    int j0 = std::max(0, static_cast<int>(std::floor(jMin)));
    int j1 = std::min(g.rows, static_cast<int>(std::ceil(jMax)));
    if (i0 >= i1 || j0 >= j1) return false; // nothing visible

    // Build the clipped grid
    out.i0 = i0;
    out.i1 = i1;
    out.j0 = j0;
    out.j1 = j1;
    out.dU = g.dU;
    out.dV = g.dV;

    out.start = { g.RowStart.x + g.dU.x * static_cast<float>(i0) + g.dV.x * static_cast<float>(j0),
                  g.RowStart.y + g.dU.y * static_cast<float>(i0) + g.dV.y * static_cast<float>(j0) };
    return true;
}
