#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/camera.h"
#include "pixel_engine/utilities/id_allocator.h"
#include "pixel_engine/core/interface/interface_sprite.h"
#include "pixel_engine/render_manager/api/culling/culling.h"

#include "core/unordered_map.h"
#include "core/vector.h"

#include <shared_mutex>
#include <atomic>

namespace pixel_engine
{
	class PERaster2D;

	typedef struct _PFE_RENDER_QUEUE_CONSTRUCT_DESC
	{
		UINT	  ScreenWidth;
		UINT	  ScreenHeight;
		Camera2D* pCamera;
		int		  TilePx;
	}PFE_RENDER_QUEUE_CONSTRUCT_DESC;

	class PFE_API PERenderQueue final : public ISingleton<PERenderQueue>
	{
		friend class ISingleton<PERenderQueue>;
	public:
		explicit PERenderQueue(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc);

		Camera2D* GetCamera() const;

		void Update(float deltaTime);
		void Render(PERaster2D* pRaster);

		bool AddSprite(PEISprite* sprite);
		bool RemoveSprite(PEISprite* sprite);
		bool RemoveSprite(UniqueId id);

	private:
		PERenderQueue() = default;

	private:
		void CreateCulling2D(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc);

		void UpdateSprite(float deltaTime);
		void RenderSprite(PERaster2D* pRaster);
		void BuildSpriteInOrder();

	private:
		std::unique_ptr<PECulling2D> m_pCulling2D{ nullptr };
		int   m_nTilePx{};
		float m_nTileStep{};

		fox::unordered_map<UniqueId, PEISprite*> m_mapSprites{};
		fox::vector<PEISprite*>					 m_ppSortedSprites{};
		std::atomic<bool> m_bDirtySprite{ true };
		UINT	  m_nScreenWidth{};
		UINT	  m_nScreenHeight{};
		Camera2D* m_pCamera{};

		mutable std::shared_mutex m_mutex;
	};
} // namespace pixel_engine
