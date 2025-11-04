#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/camera/camera.h"
#include "pixel_engine/utilities/id_allocator.h"
#include "pixel_engine/core/interface/interface_sprite.h"
#include "pixel_engine/render_manager/api/culling/culling.h"
#include "pixel_engine/core/types.h"

#include "core/unordered_map.h"
#include "core/vector.h"

#include <shared_mutex>
#include <atomic>

namespace pixel_engine
{
	class PERaster2D;

	typedef struct _PFE_CLIPPED_GRID
	{
		FVector2D start;
		FVector2D dU, dV;
		int i0, i1;
		int j0, j1;
	} PFE_CLIPPED_GRID;

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

		void Update();
		void Render(PERaster2D* pRaster);

		bool AddSprite(PEISprite* sprite);
		bool RemoveSprite(PEISprite* sprite);
		bool RemoveSprite(UniqueId id);

	private:
		PERenderQueue() = default;

		void CreateCulling2D(const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc);
		void BuildDiscreteGrid(PEISprite* sprite,
			int tilePx,
			PFE_SAMPLE_GRID_2D& out);

		void RenderSprite(PERaster2D* pRaster);
		void BuildSpriteInOrder();

		//~ Helpers
		float Det2(float ax, float ay, float bx, float by) const noexcept;
		bool InvertColumns(const FVector2D& dU, const FVector2D& dV,
			float& m00, float& m01, float& m10, float& m11) const noexcept;
		bool ClipGridToViewport(const PFE_SAMPLE_GRID_2D& g,
			int vpW, int vpH,
			PFE_CLIPPED_GRID& out) const;

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
