#pragma once

#include "PixelFoxEngineAPI.h"
#include <memory>

#include "pixel_engine/render_manager/api/buffer/image.h"
#include "pixel_engine/render_manager/components/texture/texture.h"

#include "fox_math/transform.h"
#include "fox_math/matrix.h"
#include "fox_math/vector.h"
#include "pixel_engine/core/types.h"

#include <d3d11.h>

// TODO: Create Image Raster
namespace pixel_engine
{
	typedef struct _PFE_RASTER_CONSTURCT_DESC
	{
		PFE_VIEWPORT Viewport;
		bool		 EnableBoundCheck{ true };

	} PFE_RASTER_CONSTRUCT_DESC;

	typedef struct _PFE_RASTER_INIT_DESC
	{
	} PFE_RASTER_INIT_DESC;

	/// <summary>
	/// Writes to the backbuffer
	/// </summary>
	class PFE_API PERaster2D
	{
	public:
		PERaster2D(const PFE_RASTER_CONSTRUCT_DESC* desc);
		
		bool Init(const PFE_RASTER_INIT_DESC* desc);
		void Release();

		PEImageBuffer* GetRenderTarget() const;

		void SetViewport(const PFE_VIEWPORT& rect);
		PFE_VIEWPORT GetViewport() const;

		void PutPixel(int y, int x, const PFE_FORMAT_R8G8B8_UINT& color);
		
		void DrawDiscreteQuad(
			const FVector2D& rowStart,
			const FVector2D& dU,
			const FVector2D& dV,
			int cols,
			int rows,
			const PFE_FORMAT_R8G8B8_UINT& color
		);

		void DrawDiscreteQuad(
			const FVector2D& rowStart,
			const FVector2D& dU,
			const FVector2D& dV,
			int cols,
			int rows,
			const Texture* texture
		);

		void DrawSafeQuad(
			const FVector2D& rowStart,
			const FVector2D& dU,
			const FVector2D& dV,
			int cols,
			int rows,
			const Texture* texture
		);

		bool IsBounded(unsigned x, unsigned y) const;

		void Clear(const PFE_FORMAT_R8G8B8_UINT& color);
		void Present(ID3D11DeviceContext* context, ID3D11Buffer* cpuBuffer);

	private:
		void CreateRenderTarget(const PFE_VIEWPORT& rect);

	private:
		std::unique_ptr<PEImageBuffer> m_pImageBuffer{ nullptr };
		PFE_VIEWPORT m_descViewport{ 0, 0, 0, 0 };
		bool		 m_bBoundCheck { true };
	};
} // namespace pixel_engine
