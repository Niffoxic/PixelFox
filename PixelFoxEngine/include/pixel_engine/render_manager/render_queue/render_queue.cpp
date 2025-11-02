#include "pch.h"
#include "render_queue.h"

#include "pixel_engine/render_manager/api/raster/raster.h"


pixel_engine::PERenderQueue::PERenderQueue(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc)
	:	m_nScreenHeight(desc.ScreenHeight),
		m_nScreenWidth(desc.ScreenWidth),
		m_pCamera(desc.pCamera),
		m_nTilePx(desc.TilePx)
{
	m_nTileStep = 1.f / static_cast<float>(m_nTilePx);

	CreateCulling2D(desc);
}

void pixel_engine::PERenderQueue::Update(float deltaTime)
{
	UpdateSprite(deltaTime);
}

void pixel_engine::PERenderQueue::Render(PERaster2D* pRaster)
{
	RenderSprite(pRaster);
}

bool pixel_engine::PERenderQueue::AddSprite(PEISprite* sprite)
{
	if (m_mapSprites.contains(sprite->GetInstanceID())) 
		return false;
	
	m_mapSprites[sprite->GetInstanceID()] = sprite;
	return true;
}

bool pixel_engine::PERenderQueue::RemoveSprite(PEISprite* sprite)
{
	return RemoveSprite(sprite->GetInstanceID());
}

bool pixel_engine::PERenderQueue::RemoveSprite(UniqueId id)
{
	if (not m_mapSprites.contains(id)) return false;	
	m_mapSprites.erase(id);
	return true;
}

void pixel_engine::PERenderQueue::CreateCulling2D(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc)
{
	PFE_CULL2D_CONSTRUCT_DESC cullDesc{};
	cullDesc.Viewport =
	{   0,
		0,
		static_cast<UINT>(desc.pCamera->GetViewportWidth()),
		static_cast<UINT>(desc.pCamera->GetViewportHeight())
	};
	m_pCulling2D = std::make_unique<PECulling2D>(cullDesc);
}

void pixel_engine::PERenderQueue::UpdateSprite(float deltaTime)
{
	if (m_bDirtySprite) 
	{
		m_bDirtySprite = false;
		BuildSpriteBuffer();
	}

	for (auto& sprite : m_ppSortedSprites)
	{
		sprite->Update(deltaTime, m_pCamera);
	}
}

void pixel_engine::PERenderQueue::RenderSprite(PERaster2D* pRaster)
{
	PFE_SAMPLE_GRID_2D grid{};

	for (auto& sprite : m_ppSortedSprites)
	{
		if (not sprite->IsVisible()) continue;

		if (!sprite->BuildDiscreteGrid(m_nTileStep, grid))
			continue;

		if (m_pCulling2D->ShouldCullQuad(grid.RowStart,
			grid.dU, grid.dV, grid.cols, grid.rows))
			continue;

		pRaster->DrawDiscreteQuad(
			grid.RowStart,
			grid.dU,
			grid.dV,
			grid.cols,
			grid.rows,
			{100, 100, 255} // TODO: Add Texture on Sprite
		);
	}
}

void pixel_engine::PERenderQueue::BuildSpriteBuffer()
{
	m_ppSortedSprites.clear();
	m_ppSortedSprites.reserve(m_mapSprites.size());

	for (const auto& kv : m_mapSprites)
	{
		PEISprite* s = kv.second;
		if (!s) continue;
		m_ppSortedSprites.push_back(s);
	}

	std::stable_sort(
		m_ppSortedSprites.begin(),
		m_ppSortedSprites.end(),
		[](const PEISprite* a, const PEISprite* b)
		{
			const uint32_t la = a->GetLayer();
			const uint32_t lb = b->GetLayer();
			if (la != lb) return la < lb;
			return a->GetInstanceID() < b->GetInstanceID();
		}
	);
}
