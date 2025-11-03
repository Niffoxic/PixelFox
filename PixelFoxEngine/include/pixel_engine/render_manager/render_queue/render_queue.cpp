#include "pch.h"
#include "render_queue.h"
#include "pixel_engine/render_manager/api/raster/raster.h"
#include <algorithm>

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
        BuildSpriteInOrder();
    // UpdateSprite(deltaTime); CRITICAL- THREAD GOING BOOM BOOM (must be updated by application)
}

void PERenderQueue::Render(PERaster2D* pRaster)
{
    if (m_bDirtySprite.exchange(false, std::memory_order_acq_rel))
        BuildSpriteInOrder();
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
    cullDesc.Viewport = {
        0, 0,
        static_cast<UINT>(desc.pCamera->GetViewportWidth()),
        static_cast<UINT>(desc.pCamera->GetViewportHeight())
    };
    m_pCulling2D = std::make_unique<PECulling2D>(cullDesc);
}

void PERenderQueue::UpdateSprite(float deltaTime)
{
    fox::vector<PEISprite*> snapshot;
    {
        std::shared_lock rlock(m_mutex);
        snapshot = m_ppSortedSprites;
    }
    PFE_WORLD_SPACE_DESC desc{};
    desc.pCamera    = m_pCamera;
    desc.Origin     = m_pCamera->WorldToScreen({ 0.0f, 0.0f }, m_nTilePx);
    desc.X1         = m_pCamera->WorldToScreen({ 1.0f, 0.0f }, m_nTilePx);
    desc.Y1         = m_pCamera->WorldToScreen({ 0.0f, 1.0f }, m_nTilePx);

    for (auto* sprite : snapshot)
        if (sprite) sprite->Update(deltaTime, desc);
}

void PERenderQueue::RenderSprite(PERaster2D* pRaster)
{
    PFE_SAMPLE_GRID_2D grid{};
    for (auto* sprite : m_ppSortedSprites)
    {
        if (!sprite || !sprite->IsVisible()) continue;
        if (!sprite->BuildDiscreteGrid(m_nTileStep, grid)) continue;
        if (m_pCulling2D->ShouldCullQuad(grid.RowStart, grid.dU, grid.dV, grid.cols, grid.rows)) continue;


        if (auto* texture = sprite->GetTexture())
        {
            pRaster->DrawDiscreteQuad(
                grid.RowStart, grid.dU, grid.dV, grid.cols,
                grid.rows, texture);
        }
        else
        {
            pRaster->DrawDiscreteQuad(
                grid.RowStart, grid.dU, grid.dV, grid.cols, grid.rows,
                { 100, 100, 255 });
        }
    }
}

void PERenderQueue::BuildSpriteInOrder()
{
    fox::vector<PEISprite*> local;
    {
        std::shared_lock rlock(m_mutex);
        local.reserve(m_mapSprites.size());
        for (const auto& kv : m_mapSprites)
            if (kv.second) local.push_back(kv.second);
    }

    std::stable_sort(local.begin(), local.end(),
        [](const PEISprite* a, const PEISprite* b)
        {
            const uint32_t la = a->GetLayer(), lb = b->GetLayer();
            if (la != lb) return la < lb;
            return a->GetInstanceID() < b->GetInstanceID();
        });

    {
        std::unique_lock wlock(m_mutex);
        m_ppSortedSprites.swap(local);
    }
}
