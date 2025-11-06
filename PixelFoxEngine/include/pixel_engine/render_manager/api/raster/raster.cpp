// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#include "pch.h"
#include "raster.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <d3d11.h>

#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_engine;

_Use_decl_annotations_
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

_Use_decl_annotations_
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

_Use_decl_annotations_
void PERaster2D::SetViewport(const PFE_VIEWPORT& rect)
{
    if (rect == m_descViewport) return;
    m_descViewport = rect;
    CreateRenderTarget(m_descViewport);
}

_Use_decl_annotations_
void PERaster2D::PutPixel(int y, int x, const PFE_FORMAT_R8G8B8_UINT& color)
{
    if (!m_pImageBuffer) return;
    if (m_bBoundCheck && !IsBounded(x, y)) return;
    m_pImageBuffer->WriteAt(y, x, color);
}

_Use_decl_annotations_
void pixel_engine::PERaster2D::DrawQuadColor(const PFE_RASTER_DRAW_CMD& cmd)
{
    auto startFrom = cmd.startBase;
    for (int j = 0; j < cmd.totalRows; ++j)
    {
        FVector2D p = startFrom;
        for (int i = 0; i < cmd.totalColumns; ++i)
        {
            const int ix = static_cast<int>(std::lround(p.x));
            const int iy = static_cast<int>(std::lround(p.y));

            if (IsBounded(ix, iy))
            {
                m_pImageBuffer->WriteAt(iy, ix, cmd.color);
            }
            p.x += cmd.deltaAxisU.x;
            p.y += cmd.deltaAxisU.y;
        }
        startFrom.x += cmd.deltaAxisV.x;
        startFrom.y += cmd.deltaAxisV.y;
    }
}

_Use_decl_annotations_
void pixel_engine::PERaster2D::DrawQuadTile(const PFE_RASTER_DRAW_CMD& cmd)
{
    auto startFrom = cmd.startBase;
    for (int j = 0; j < cmd.totalRows; ++j)
    {
        FVector2D p = startFrom;
        for (int i = 0; i < cmd.totalColumns; ++i)
        {
            const int ix = static_cast<int>(std::lround(p.x));
            const int iy = static_cast<int>(std::lround(p.y));

            if (IsBounded(ix, iy))
            {
                auto color = cmd.sampledTexture->GetPixel(i, j);
                
                if (!color.IsBlack())
                {
                    m_pImageBuffer->WriteAt(iy, ix, color);
                }
            }

            p.x += cmd.deltaAxisU.x;
            p.y += cmd.deltaAxisU.y;
        }
        startFrom.x += cmd.deltaAxisV.x;
        startFrom.y += cmd.deltaAxisV.y;
    }
}

_Use_decl_annotations_
void pixel_engine::PERaster2D::DrawQuadBackground(const PFE_RASTER_DRAW_CMD& cmd)
{
    if (!m_pScheduler || !m_pImageBuffer || !cmd.sampledTexture) return;

    pixel_engine::RASTERIZE_TASK_DESC proto{};
    proto.target          = m_pImageBuffer.get();
    proto.sampledTexture  = cmd.sampledTexture;
    proto.deltaAxisU      = cmd.deltaAxisU;
    proto.deltaAxisV      = cmd.deltaAxisV;
    proto.columnStartFrom = cmd.columnStartFrom;
    proto.columneEndAt    = cmd.columneEndAt;
    proto.rowOffset       = cmd.rowStartFrom;
    proto.totalColumns    = cmd.totalColumns;
    proto.totalRows       = cmd.totalColumns;
    proto.TexWidth        = cmd.sampledTexture->GetWidth();
    proto.TexHeight       = cmd.sampledTexture->GetHeight();

    proto.startBase =
    {
        // x
        cmd.startBase.x                         + 
        static_cast<float>(cmd.columnStartFrom) *
        cmd.deltaAxisU.x                        +
        static_cast<float>(cmd.rowStartFrom)    *
        cmd.deltaAxisV.x,

        // y
        cmd.startBase.y                         +
        static_cast<float>(cmd.columnStartFrom) *
        cmd.deltaAxisU.y                        +
        static_cast<float>(cmd.rowStartFrom)    *
        cmd.deltaAxisV.y
    };

    constexpr int CHUNK_ROWS = 32;

    const int totalRows = cmd.rowEndAt - cmd.rowStartFrom;
    const int numJobs   = (totalRows + CHUNK_ROWS - 1) / CHUNK_ROWS;

    for (int k = 0; k < numJobs; ++k)
    {
        const int relA = k * CHUNK_ROWS;
        const int relB = std::min(totalRows, relA + CHUNK_ROWS);

        auto desc = proto;
        desc.rowStartFrom   = relA;
        desc.rowEndAt   = relB;

        m_pScheduler->Enqueue(pixel_engine::PERasterizeTask{ desc });
    }

    m_pScheduler->Dispatch();
    m_pScheduler->Wait    ();
}

_Use_decl_annotations_
bool PERaster2D::IsBounded(int x, int y) const noexcept
{
    if (!m_bBoundCheck) return true;

    const int W = static_cast<int>(m_descViewport.w);
    const int H = static_cast<int>(m_descViewport.h);

    return  (static_cast<unsigned>(x) < static_cast<unsigned>(W)) &&
            (static_cast<unsigned>(y) < static_cast<unsigned>(H));
}

_Use_decl_annotations_
void PERaster2D::Clear(const PFE_FORMAT_R8G8B8_UINT& color)
{
    if (m_pImageBuffer) m_pImageBuffer->ClearImageBuffer(color);
}

_Use_decl_annotations_
void PERaster2D::Present(ID3D11DeviceContext* context, ID3D11Buffer* cpuBuffer)
{
    context->UpdateSubresource(
        cpuBuffer, 0, nullptr,
        m_pImageBuffer->Data(),
        (UINT)m_pImageBuffer->RowPitch(), 0);
}

_Use_decl_annotations_
void PERaster2D::CreateRenderTarget(const PFE_VIEWPORT& rect)
{
    if (m_pImageBuffer) m_pImageBuffer.reset();

    PE_IMAGE_BUFFER_DESC imageDesc{};
    imageDesc.Height = rect.h - rect.y;
    imageDesc.Width  = rect.w - rect.x;

    m_pImageBuffer = std::make_unique<PEImageBuffer>(imageDesc);
}
