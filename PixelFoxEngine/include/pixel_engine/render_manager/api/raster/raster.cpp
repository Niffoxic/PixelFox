#include "pch.h"
#include "raster.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <d3d11.h>

#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

static inline int Clampi(int v, int lo, int hi) noexcept { return (v < lo) ? lo : (v > hi ? hi : v); }
static inline int SampleIndexCenter(float x) noexcept    { return static_cast<int>(std::floor(x + 0.5f)); }

PERaster2D::PERaster2D(const PFE_RASTER_CONSTRUCT_DESC* desc)
{
    m_descViewport = desc->Viewport;
    m_bBoundCheck  = desc->EnableBoundCheck;
    CreateRenderTarget(m_descViewport);
}

pixel_engine::PERaster2D::~PERaster2D()
{
    if (m_pImageBuffer)
    { 
        m_pImageBuffer.reset();
        m_pImageBuffer = nullptr;
    }
}

bool PERaster2D::Init(const PFE_RASTER_INIT_DESC* desc)
{
    m_pScheduler = std::make_unique<RasterizeScheduler>();
    m_pScheduler->Initialize(desc->WorkerCount);
    return true;
}

void PERaster2D::Release()
{
    if (m_pImageBuffer)
    {
        m_pImageBuffer.reset();
        m_pImageBuffer = nullptr;
    }
}

void PERaster2D::SetViewport(const PFE_VIEWPORT& rect)
{
    if (rect == m_descViewport) return;
    m_descViewport = rect;
    CreateRenderTarget(m_descViewport);
}

void PERaster2D::PutPixel(int y, int x, const PFE_FORMAT_R8G8B8_UINT& color)
{
    if (!m_pImageBuffer) return;
    if (m_bBoundCheck && !IsBounded(x, y)) return;
    m_pImageBuffer->WriteAt(y, x, color);
}

void pixel_engine::PERaster2D::DrawDiscreteQuad(
    const FVector2D& rowStart,
    const FVector2D& dU,
    const FVector2D& dV,
    int cols, int rows,
    const PFE_FORMAT_R8G8B8_UINT& color)
{
    auto startFrom = rowStart;
    for (int j = 0; j < rows; ++j)
    {
        FVector2D p = startFrom;
        for (int i = 0; i < cols; ++i)
        {
            const int ix = static_cast<int>(std::lround(p.x));
            const int iy = static_cast<int>(std::lround(p.y));
            if (IsBounded(ix, iy))
            {
                m_pImageBuffer->WriteAt(iy, ix, color);
            }
            p.x += dU.x; p.y += dU.y;
        }
        startFrom.x += dV.x; startFrom.y += dV.y;
    }
}

void pixel_engine::PERaster2D::DrawDiscreteQuad(
    const FVector2D& rowStart,
    const FVector2D& dU, const FVector2D& dV,
    int cols, int rows, const pixel_engine::Texture* tex)
{
    auto startFrom = rowStart;
    for (int j = 0; j < rows; ++j)
    {
        FVector2D p = startFrom;
        for (int i = 0; i < cols; ++i)
        {
            const int ix = static_cast<int>(std::lround(p.x));
            const int iy = static_cast<int>(std::lround(p.y));
            if (IsBounded(ix, iy))
            {
                m_pImageBuffer->WriteAt(iy, ix, tex->GetPixel(i, j));
            }
            p.x += dU.x; p.y += dU.y;
        }
        startFrom.x += dV.x; startFrom.y += dV.y;
    }
}

void PERaster2D::DrawQuadClippedMT(
    const FVector2D& start,
    const FVector2D& dU,
    const FVector2D& dV,
    int i0, int i1,
    int j0, int j1,
    int colsTotal, int rowsTotal,
    const pixel_engine::Texture* tex)
{
    // Basic guards
    if (!m_pScheduler || !m_pImageBuffer || !tex) return;

    pixel_engine::RasterizeTaskDesc proto{};
    proto.Target = m_pImageBuffer.get();
    proto.Texture = tex;
    proto.dU = dU;
    proto.dV = dV;
    proto.i0 = i0;
    proto.i1 = i1;
    proto.j0Abs = j0;
    proto.ColsTotal = colsTotal;
    proto.RowsTotal = rowsTotal;
    proto.TexW = static_cast<int>(tex->GetWidth());
    proto.TexH = static_cast<int>(tex->GetHeight());

    proto.StartBase = 
    {
        start.x + static_cast<float>(i0) * dU.x + static_cast<float>(j0) * dV.x,
        start.y + static_cast<float>(i0) * dU.y + static_cast<float>(j0) * dV.y
    };

    constexpr int CHUNK_ROWS = 16;
    const int totalRows = j1 - j0;
    const int numJobs = (totalRows + CHUNK_ROWS - 1) / CHUNK_ROWS;

    for (int k = 0; k < numJobs; ++k)
    {
        const int relA = k * CHUNK_ROWS;
        const int relB = std::min(totalRows, relA + CHUNK_ROWS);

        auto desc = proto;
        desc.jA = relA;
        desc.jB = relB;

        m_pScheduler->Enqueue(pixel_engine::PERasterizeTask{ desc });
    }

    m_pScheduler->Dispatch();
    m_pScheduler->Wait();
}

bool PERaster2D::IsBounded(int x, int y) const noexcept
{
    if (!m_bBoundCheck) return true;

    const int W = static_cast<int>(m_descViewport.w);
    const int H = static_cast<int>(m_descViewport.h);

    return  (static_cast<unsigned>(x) < static_cast<unsigned>(W)) &&
            (static_cast<unsigned>(y) < static_cast<unsigned>(H));
}

void PERaster2D::Clear(const PFE_FORMAT_R8G8B8_UINT& color)
{
    if (m_pImageBuffer) m_pImageBuffer->ClearImageBuffer(color);
}

void PERaster2D::Present(ID3D11DeviceContext* context, ID3D11Buffer* cpuBuffer)
{
    context->UpdateSubresource(
        cpuBuffer, 0, nullptr,
        m_pImageBuffer->Data(),
        (UINT)m_pImageBuffer->RowPitch(), 0);
}

void PERaster2D::CreateRenderTarget(const PFE_VIEWPORT& rect)
{
    if (m_pImageBuffer) m_pImageBuffer.reset();

    PE_IMAGE_BUFFER_DESC imageDesc{};
    imageDesc.Height = rect.h - rect.y;
    imageDesc.Width = rect.w - rect.x;

    m_pImageBuffer = std::make_unique<PEImageBuffer>(imageDesc);
}
