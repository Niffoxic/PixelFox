#include "pch.h"
#include "culling.h"

#include "pixel_engine/exceptions/base_exception.h"

pixel_engine::PECulling2D::PECulling2D(const PFE_CULL2D_CONSTRUCT_DESC& desc)
{
    m_viewport = desc.Viewport;
}

void pixel_engine::PECulling2D::Init(const pixel_engine::PFE_VIEWPORT& viewport)
{
    m_viewport = viewport;
}

bool pixel_engine::PECulling2D::ShouldCullQuad(const PFE_SAMPLE_GRID_2D& grid) const noexcept
{
    if (grid.cols <= 0 || grid.rows <= 0) return true;

    const PFE_AABB2D aabb = ComputeQuadAABB(grid);

    const float vpw = static_cast<float>(m_viewport.w);
    const float vph = static_cast<float>(m_viewport.h);

    if (aabb.maxX < 0.0f || aabb.maxY < 0.0f) return true;
    if (aabb.minX >= vpw || aabb.minY >= vph) return true;

    return false;
}

pixel_engine::PFE_AABB2D pixel_engine::PECulling2D::ComputeQuadAABB(const PFE_SAMPLE_GRID_2D& grid) const noexcept
{
    const FVector2D A = grid.RowStart; // left-top

    // right-top
    const FVector2D B{ grid.RowStart.x + grid.dU.x * static_cast<float>(grid.cols),
                       grid.RowStart.y + grid.dU.y * static_cast<float>(grid.cols) };

    // left-bottom
    const FVector2D C{ grid.RowStart.x + grid.dV.x * static_cast<float>(grid.rows),
                       grid.RowStart.y + grid.dV.y * static_cast<float>(grid.rows) };

    // right-bottom
    const FVector2D D{ B.x + grid.dV.x * static_cast<float>(grid.rows),
                       B.y + grid.dV.y * static_cast<float>(grid.rows) };

    constexpr float pad = 0.5f;

    PFE_AABB2D out{};
    out.minX = std::min(std::min(A.x, B.x), std::min(C.x, D.x)) - pad;
    out.maxX = std::max(std::max(A.x, B.x), std::max(C.x, D.x)) + pad;
    out.minY = std::min(std::min(A.y, B.y), std::min(C.y, D.y)) - pad;
    out.maxY = std::max(std::max(A.y, B.y), std::max(C.y, D.y)) + pad;

    return out;
}

void pixel_engine::PECulling2D::SetViewport(const pixel_engine::PFE_VIEWPORT& viewport) noexcept
{
    m_viewport = viewport;
}

UINT pixel_engine::PECulling2D::GetViewportWidth()  const noexcept
{
    return m_viewport.w;
}

UINT pixel_engine::PECulling2D::GetViewportHeight() const noexcept
{
    return m_viewport.h;
}