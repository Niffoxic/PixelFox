// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/interface/interface_singleton.h"
#include "pixel_engine/render_manager/components/camera/camera.h"
#include "pixel_engine/render_manager/components/font/font.h"
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
		_In_ FVector2D start;
		_In_ FVector2D deltaAxisU, deltaAxisV;
		_In_ int columnStartFrom;
		_In_ int columneEndAt;
		_In_ int j0;
		_In_ int j1;
	} PFE_CLIPPED_GRID;

	typedef struct _PFE_RENDER_QUEUE_CONSTRUCT_DESC
	{
		_In_ UINT	   ScreenWidth;
		_In_ UINT	   ScreenHeight;
		_In_ Camera2D* pCamera;
		_In_ int	   TilePx;
	}PFE_RENDER_QUEUE_CONSTRUCT_DESC;

	class PFE_API PERenderQueue final : public ISingleton<PERenderQueue>
	{
		friend class ISingleton<PERenderQueue>;
	public:
		explicit PERenderQueue(_In_ const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc);

		_NODISCARD _Check_return_ _Success_(return != nullptr)
		Camera2D* GetCamera() const;

		void Update();
		void Render(_Inout_ PERaster2D* pRaster);

		bool AddSprite   (_Inout_ PEISprite* sprite);
		bool RemoveSprite(_Inout_ PEISprite* sprite);
		bool RemoveSprite(_In_ UniqueId id);

		bool AddFont   (_Inout_ PEFont* font);
		bool RemoveFont(_Inout_ PEFont* font);
		bool RemoveFont(_In_ UniqueId id);

		void EnableFPS(bool flag, const FVector2D& position)
		{
			m_bShowFPS    = flag;
			m_fpsPosition = position;
		}

		void SetFPSPx(int px ) { m_nFontPx = px; }
		int  GetFPSPx() const { return m_nFontPx; }

		bool IsShowFPS			() const { return m_bShowFPS;	 }
		FVector2D GetFPSPosition() const { return m_fpsPosition; }

	private:
		PERenderQueue() = default;

		//~ Render Sprite
		void CreateCulling2D(_In_ const PFE_RENDER_QUEUE_CONSTRUCT_DESC& desc);
		
		void BuildDiscreteGrid(
			_Inout_ PEISprite*			sprite,
			_Inout_ Texture*			sampledTexture,
			_In_    int				    tilePx,
			_Out_   PFE_SAMPLE_GRID_2D& out);

		void RenderSprite	   (_Inout_ PERaster2D* pRaster);
		void BuildSpriteInOrder();

		//~ Render Font
		void RenderFont(_Inout_ PERaster2D* pRaster);

		//~ Helpers
		float Det2(
			_In_ float ax,
			_In_ float ay,
			_In_ float bx,
			_In_ float by) const noexcept;
		
		bool InvertColumns(
			_In_ const FVector2D& deltaAxisU,
			_In_ const FVector2D& deltaAxisV,
			_Out_ float& m00,
			_Out_ float& m01,
			_Out_ float& m10,
			_Out_ float& m11) const noexcept;

		bool ClipGridToViewport(
			_In_  const PFE_SAMPLE_GRID_2D& grid,
			_In_  int						vpW,
			_In_  int						vpH,
			_Out_ PFE_CLIPPED_GRID&			out) const;

	private:
		void ApplyPending();

	private:
		bool	  m_bShowFPS	{ false };
		FVector2D m_fpsPosition	{34, 10};
		int		  m_nFontPx	    { 16 };
		int		  m_nFrameCount { 0 };
		int		  m_nlastFps	{ 0 };

		std::unique_ptr<PECulling2D> m_pCulling2D{ nullptr };

		int   m_nTilePx  {};
		float m_nTileStep{};

		//~ Render Sprite
		fox::unordered_map<UniqueId, PEISprite*> m_mapSprites{};
		fox::vector<PEISprite*>					 m_ppSortedSprites{};
		
		std::atomic<bool> m_bDirtySprite { true };
		UINT			  m_nScreenWidth { 0u };
		UINT			  m_nScreenHeight{ 0u };
		Camera2D*		  m_pCamera		 { nullptr };

		//~ Render Font
		fox::unordered_map<UniqueId, PEFont*> m_mapFonts{};

		//~ manage adding 
		fox::vector<PEISprite*> m_pendingAdd{};
		fox::vector<UniqueId>   m_pendingRemove{};
	};
} // namespace pixel_engine
