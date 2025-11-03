#include "pch.h"
#include "raster.h"

#include <algorithm>
#include <cmath>
#include <limits>

using namespace pixel_engine;

static inline pixel_engine::PFE_FORMAT_R8G8B8_UINT
FetchTexelNearestFast(const pixel_engine::Texture* tex, int sx, int sy)
{
    using pixel_engine::TextureFormat;
    const uint32_t bpp = tex->BytesPerPixel();
    const uint32_t pitch = tex->GetRowStride();
    const auto fmt = tex->GetFormat();
    const auto data = tex->Data();

    const size_t off = static_cast<size_t>(sy) * pitch + static_cast<size_t>(sx) * bpp;
    const uint8_t* p = data.data + off;

    switch (fmt)
    {
    case TextureFormat::R8:    return { p[0], p[0], p[0] };
    case TextureFormat::RG8:   return { p[0], p[1], 0u };
    case TextureFormat::RGB8:  return { p[0], p[1], p[2] };
    case TextureFormat::RGBA8: return { p[0], p[1], p[2] };
    default:                   return { 0, 0, 0 };
    }
}

PERaster2D::PERaster2D(const PFE_RASTER_CONSTRUCT_DESC* desc)
{
	m_descViewport = desc->Viewport;
	m_bBoundCheck  = desc->EnableBoundCheck;
	CreateRenderTarget(m_descViewport);
}

bool PERaster2D::Init(const PFE_RASTER_INIT_DESC* desc)
{
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

PEImageBuffer* PERaster2D::GetRenderTarget() const
{
	return m_pImageBuffer.get();
}

void PERaster2D::SetViewport(const PFE_VIEWPORT& rect)
{
	if (rect == m_descViewport) return;
	m_descViewport = rect;
	CreateRenderTarget(m_descViewport);
}

PFE_VIEWPORT PERaster2D::GetViewport() const
{
	return m_descViewport;
}

void PERaster2D::PutPixel(int y, int x, const PFE_FORMAT_R8G8B8_UINT& color)
{
	if (not m_pImageBuffer) return;
	if (m_bBoundCheck) 
	{
		if (not IsBounded(x, y)) return;
	}
	m_pImageBuffer->WriteAt(y, x, color);
}

void PERaster2D::DrawDiscreteQuad(
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
    const FVector2D& dU,
    const FVector2D& dV,
    int cols, int rows,
    const Texture* texture)
{
    if (!texture || cols <= 0 || rows <= 0) return;

    const int texW = static_cast<int>(texture->GetWidth());
    const int texH = static_cast<int>(texture->GetHeight());
    if (texW <= 0 || texH <= 0) return;

    const double x0 = rowStart.x;
    const double y0 = rowStart.y;
    const double x1 = x0 + dU.x * cols;
    const double y1 = y0 + dU.y * cols;
    const double x2 = x0 + dV.x * rows;
    const double y2 = y0 + dV.y * rows;
    const double x3 = x0 + dU.x * cols + dV.x * rows;
    const double y3 = y0 + dU.y * cols + dV.y * rows;

    int minX = static_cast<int>(std::floor(std::min({ x0, x1, x2, x3 })));
    int maxX = static_cast<int>(std::ceil(std::max({ x0, x1, x2, x3 })));
    int minY = static_cast<int>(std::floor(std::min({ y0, y1, y2, y3 })));
    int maxY = static_cast<int>(std::ceil(std::max({ y0, y1, y2, y3 })));

    minX = std::max(minX, 0);
    minY = std::max(minY, 0);
    maxX = std::min(maxX, static_cast<int>(m_descViewport.w - 1));
    maxY = std::min(maxY, static_cast<int>(m_descViewport.h - 1));
    if (minX > maxX || minY > maxY) return;

    const double a = dU.x, b = dV.x;
    const double c = dU.y, d = dV.y;
    const double det = a * d - b * c;
    if (std::abs(det) < 1e-8) {
        auto startFrom = rowStart;
        for (int j = 0; j < rows; ++j) {
            FVector2D p = startFrom;
            for (int i = 0; i < cols; ++i) {
                const int ix = static_cast<int>(std::lround(p.x));
                const int iy = static_cast<int>(std::lround(p.y));
                if (IsBounded(ix, iy)) {
                    const int tx = std::clamp(i, 0, texW - 1);
                    const int ty = std::clamp(j, 0, texH - 1);
                    auto texel = texture->GetPixel(static_cast<uint32_t>(tx),
                        static_cast<uint32_t>(ty));
                    m_pImageBuffer->WriteAt(iy, ix, texel);
                }
                p.x += dU.x; p.y += dU.y;
            }
            startFrom.x += dV.x; startFrom.y += dV.y;
        }
        return;
    }

    const double inv00 = d / det;
    const double inv01 = -b / det;
    const double inv10 = -c / det;
    const double inv11 = a / det;

    const double ox = -x0;
    const double oy = -y0;

    for (int y = minY; y <= maxY; ++y)
    {
        const double py = (static_cast<double>(y) + 0.5);
        const double ry = oy + py;

        for (int x = minX; x <= maxX; ++x) {
            const double px = (static_cast<double>(x) + 0.5);
            const double rx = ox + px;

            const double u = inv00 * rx + inv01 * ry;
            const double v = inv10 * rx + inv11 * ry;

            if (u >= -0.5 && u < static_cast<double>(cols) - 0.5 &&
                v >= -0.5 && v < static_cast<double>(rows) - 0.5)
            {
                int tx = static_cast<int>(std::floor(u + 0.5));
                int ty = static_cast<int>(std::floor(v + 0.5));
                tx = std::clamp(tx, 0, texW - 1);
                ty = std::clamp(ty, 0, texH - 1);

                auto texel = texture->GetPixel(static_cast<uint32_t>(tx),
                    static_cast<uint32_t>(ty));
                m_pImageBuffer->WriteAt(y, x, texel);
            }
        }
    }
}

void pixel_engine::PERaster2D::DrawSafeQuad(const FVector2D& rowStart,
    const FVector2D& dU, const FVector2D& dV,
    int cols, int rows,
    const Texture* texture)
{
    auto startFrom = rowStart;
    for (int j = 0; j < texture->GetHeight(); ++j)
    {
        FVector2D p = startFrom;
        for (int i = 0; i < texture->GetWidth(); ++i)
        {
            const int ix = static_cast<int>(std::lround(rowStart.x));
            const int iy = static_cast<int>(std::lround(rowStart.y));
            if (IsBounded(ix, iy))
            {
                m_pImageBuffer->WriteAt(iy + j, ix, texture->GetPixel(i, j));
            }
            p.x += dU.x; p.y += dU.y;
        }
        startFrom.x += dV.x; startFrom.y += dV.y;
    }
}

bool pixel_engine::PERaster2D::IsBounded(unsigned x, unsigned y) const
{
	if (not m_bBoundCheck) return true;
	UINT width = m_descViewport.w - m_descViewport.x;
	UINT height = m_descViewport.h - m_descViewport.y;

	return x < width && y < height;
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

void pixel_engine::PERaster2D::CreateRenderTarget(const PFE_VIEWPORT& rect)
{
	if (m_pImageBuffer) m_pImageBuffer.reset();

	PE_IMAGE_BUFFER_DESC imageDesc{};
	imageDesc.Height = rect.h - rect.y;
	imageDesc.Width  = rect.w - rect.x;

	m_pImageBuffer = std::make_unique<PEImageBuffer>(imageDesc);
}
