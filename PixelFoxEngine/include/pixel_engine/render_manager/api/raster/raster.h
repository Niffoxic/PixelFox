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
    typedef struct _PFE_RASTER_CONSTRUCT_DESC
    {
        _In_ PFE_VIEWPORT Viewport        {};
        _In_ bool         EnableBoundCheck{ true };
    } PFE_RASTER_CONSTRUCT_DESC;

    typedef struct _PFE_RASTER_INIT_DESC
    {
        _In_ unsigned WorkerCount{ 4u };
    } PFE_RASTER_INIT_DESC;

    typedef struct _PFE_RASTER_DRAW_CMD
    {
        _In_ const FVector2D& startBase;
        _In_ const FVector2D& deltaAxisU;
        _In_ const FVector2D& deltaAxisV;
        _In_ int              columnStartFrom;
        _In_ int              columneEndAt;
        _In_ int              rowStartFrom;
        _In_ int              rowEndAt;
        _In_ int              totalColumns;
        _In_ int              totalRows;
        _In_ const Texture*   sampledTexture;

        //~ rare case if sampled texture is failed
        _In_ const PFE_FORMAT_R8G8B8_UINT& color;
    } PFE_RASTER_DRAW_CMD;

    class PFE_API PERaster2D
    {
    public:
        explicit PERaster2D(_In_ const PFE_RASTER_CONSTRUCT_DESC* desc);
        ~PERaster2D();

        PERaster2D(_In_ const PERaster2D&) = delete;
        PERaster2D(_Inout_ PERaster2D&&)   = delete;

        PERaster2D& operator=(_In_ const PERaster2D&) = delete;
        PERaster2D& operator=(_Inout_ PERaster2D&&)      = delete;

        _NODISCARD _Check_return_
        bool Init(_In_ const PFE_RASTER_INIT_DESC* desc);
        
        void Release();

        void SetViewport(_In_ const PFE_VIEWPORT& rect);

        _NODISCARD _Check_return_ _Notnull_
        PEImageBuffer* GetRenderTarget() const noexcept { return m_pImageBuffer.get(); }

        _NODISCARD _Check_return_
        PFE_VIEWPORT GetViewport() const noexcept { return m_descViewport; }

        void Clear(_In_ const PFE_FORMAT_R8G8B8_UINT& color);

        void PutPixel(
            _In_ int y,
            _In_ int x,
            _In_ const PFE_FORMAT_R8G8B8_UINT& color);

        void DrawQuadColor     (_In_ const PFE_RASTER_DRAW_CMD& cmd);
        void DrawQuadTile      (_In_ const PFE_RASTER_DRAW_CMD& cmd);
        void DrawQuadBackground(_In_ const PFE_RASTER_DRAW_CMD& cmd);

        void Present(
            _In_ ID3D11DeviceContext* context,
            _In_ ID3D11Buffer* cpuBuffer);

        _NODISCARD _Check_return_
        bool IsBounded(
            _In_ int x,
            _In_ int y) const noexcept;

        void SetBoundCheck(_In_ bool enabled) noexcept { m_bBoundCheck = enabled; }
        
        _NODISCARD _Check_return_
        bool GetBoundCheck() const noexcept { return m_bBoundCheck; }

    private:
        void CreateRenderTarget(_In_ const PFE_VIEWPORT& rect);

    private:
        std::unique_ptr<PEImageBuffer>      m_pImageBuffer{ nullptr };
        std::unique_ptr<RasterizeScheduler> m_pScheduler  { nullptr };

        PFE_VIEWPORT                   m_descViewport{ 0, 0, 0, 0 };
        bool                           m_bBoundCheck { true };
    };
} // namespace pixel_engine
