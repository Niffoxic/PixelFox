#pragma once

#include "PixelFoxEngineAPI.h"

#include "pixel_engine/core/types.h"
#include <algorithm>

#include "fox_math/matrix.h"
#include "fox_math/transform.h"
#include "fox_math/vector.h"


namespace pixel_engine
{

    struct PFE_CULL2D_CONSTRUCT_DESC
    {
        PFE_VIEWPORT Viewport;
    };

    class PFE_API PECulling2D
    {
    public:
        explicit PECulling2D(const PFE_CULL2D_CONSTRUCT_DESC& desc);

        void Init(const PFE_VIEWPORT& viewport);
        void Release();

        void SetViewport(const PFE_VIEWPORT& viewport) noexcept;

        UINT GetViewportWidth()  const noexcept;
        UINT GetViewportHeight() const noexcept;

        bool ShouldCullQuad       (const PFE_SAMPLE_GRID_2D& grid) const noexcept;
        PFE_AABB2D ComputeQuadAABB(const PFE_SAMPLE_GRID_2D& grid) const noexcept;

    private:
        PFE_VIEWPORT m_viewport{ 0, 0, 1280, 720 };
    };
} // namespace pixel_engine
