#include "pch.h"
#include "culling.h"

pixel_engine::PECulling2D::PECulling2D(const PFE_CULL2D_CONSTRUCT_DESC* desc)
{
    m_viewport = desc->Viewport;
}

void pixel_engine::PECulling2D::Init(const pixel_engine::PFE_VIEWPORT& viewport)
{
    m_viewport = viewport;
}

bool pixel_engine::PECulling2D::ShouldCullQuad(
    const FVector2D& rowStart,
    const FVector2D& dU, 
    const FVector2D& dV,
    int cols, int rows) const noexcept
{
    if (cols <= 0 || rows <= 0) return true;

    const PFE_AABB2D aabb = ComputeQuadAABB(rowStart, dU, dV, cols, rows);

    const float vpw = static_cast<float>(m_viewport.w);
    const float vph = static_cast<float>(m_viewport.h);

    if (aabb.maxX < 0.0f || aabb.maxY < 0.0f) return true;
    if (aabb.minX >= vpw || aabb.minY >= vph) return true;

    return false;
}

pixel_engine::PFE_AABB2D pixel_engine::PECulling2D::ComputeQuadAABB(
    const FVector2D& rowStart, 
    const FVector2D& dU,
    const FVector2D& dV,
    int cols, int rows) const noexcept
{
    const FVector2D A = rowStart;

    const FVector2D B{ rowStart.x + dU.x * static_cast<float>(cols),
                       rowStart.y + dU.y * static_cast<float>(cols) };
    const FVector2D C{ rowStart.x + dV.x * static_cast<float>(rows),
                       rowStart.y + dV.y * static_cast<float>(rows) };
    const FVector2D D{ B.x + dV.x * static_cast<float>(rows),
                       B.y + dV.y * static_cast<float>(rows) };

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