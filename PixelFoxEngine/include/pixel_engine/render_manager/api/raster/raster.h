#pragma once

#include "PixelFoxEngineAPI.h"
#include <memory>

#include "pixel_engine/render_manager/api/buffer/image.h"
#include "pixel_engine/render_manager/components/texture/texture.h"
#include "pixel_engine/core/types.h"

#include "fox_math/transform.h"
#include "fox_math/matrix.h"
#include "fox_math/vector.h"

#include "task/raster_scheduler.h"

#include <d3d11.h>


namespace pixel_engine
{
    struct PFE_RASTER_CONSTRUCT_DESC
    {
        PFE_VIEWPORT Viewport{};
        bool         EnableBoundCheck{ true };
    };

    struct PFE_RASTER_INIT_DESC
    {
        unsigned WorkerCount{ 4u };
    };

    class PFE_API PERaster2D
    {
    public:
        explicit PERaster2D(const PFE_RASTER_CONSTRUCT_DESC* desc);
        ~PERaster2D();

        PERaster2D(const PERaster2D&) = delete;
        PERaster2D& operator=(const PERaster2D&) = delete;

        bool Init(const PFE_RASTER_INIT_DESC* desc);
        void Release();

        PEImageBuffer* GetRenderTarget() const noexcept { return m_pImageBuffer.get(); }

        void         SetViewport(const PFE_VIEWPORT& rect);
        PFE_VIEWPORT GetViewport() const noexcept { return m_descViewport; }

        void Clear(const PFE_FORMAT_R8G8B8_UINT& color);
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
            const pixel_engine::Texture* tex
        );

        void DrawQuadClippedMT(const FVector2D& start,
            const FVector2D& dU,
            const FVector2D& dV,
            int i0, int i1, int j0, int j1,
            int colsTotal, int rowsTotal,
            const pixel_engine::Texture* tex);

        void Present(ID3D11DeviceContext* context, ID3D11Buffer* cpuBuffer);

        bool IsBounded    (int x, int y) const noexcept;
        void SetBoundCheck(bool enabled)       noexcept { m_bBoundCheck = enabled;  }
        bool GetBoundCheck()             const noexcept { return m_bBoundCheck;     }

    private:
        void CreateRenderTarget(const PFE_VIEWPORT& rect);

    private:
        std::unique_ptr<PEImageBuffer> m_pImageBuffer{ nullptr };
        std::unique_ptr<RasterizeScheduler> m_pScheduler{ nullptr };

        PFE_VIEWPORT                   m_descViewport{ 0, 0, 0, 0 };
        bool                           m_bBoundCheck { true };
    };
} // namespace pixel_engine
