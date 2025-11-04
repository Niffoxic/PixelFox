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

#include "pixel_engine/core/types.h"
#include <algorithm>

#include "fox_math/matrix.h"
#include "fox_math/transform.h"
#include "fox_math/vector.h"

namespace pixel_engine
{
    typedef struct _PFE_CULL2D_CONSTRUCT_DESC
    {
        _In_ PFE_VIEWPORT Viewport;
    } PFE_CULL2D_CONSTRUCT_DESC;

    class PFE_API PECulling2D
    {
    public:
        explicit PECulling2D(_In_ const PFE_CULL2D_CONSTRUCT_DESC& desc);

        void Init   (_In_ const PFE_VIEWPORT& viewport);
        void Release();

        void SetViewport(_In_ const PFE_VIEWPORT& viewport) noexcept;

        _NODISCARD _Check_return_
        UINT GetViewportWidth () const noexcept;
        
        _NODISCARD _Check_return_
        UINT GetViewportHeight() const noexcept;

        _NODISCARD _Check_return_
        bool ShouldCullQuad       (_In_ const PFE_SAMPLE_GRID_2D& grid) const noexcept;
        
        _NODISCARD _Check_return_
        PFE_AABB2D ComputeQuadAABB(_In_ const PFE_SAMPLE_GRID_2D& grid) const noexcept;

    private:
        PFE_VIEWPORT m_viewport{ 0, 0, 1280, 720 }; // just a default
    };
} // namespace pixel_engine
