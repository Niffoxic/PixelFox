#include "pch.h"
#include "raster.h"

#include <algorithm>
#include <cmath>

using namespace pixel_engine;

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

	const uint32_t texW = texture->GetWidth();
	const uint32_t texH = texture->GetHeight();
	if (texW == 0 || texH == 0) return;

	auto startFrom = rowStart;

	// Precompute normalization
	const float invCols = 1.0f / static_cast<float>(cols);
	const float invRows = 1.0f / static_cast<float>(rows);

	const bool flipV = (texture->GetOrigin() == Origin::BottomLeft);

	for (int j = 0; j < rows; ++j)
	{
		// Normalized v
		float v		  = (static_cast<float>(j) + 0.5f) * invRows;
		v			  = std::min(v, 1.0f - std::numeric_limits<float>::epsilon());
		int ty		  = static_cast<int>(v * static_cast<float>(texH));
		ty			  = std::clamp(ty, 0, static_cast<int>(texH) - 1);
		if (flipV) ty = static_cast<int>(texH) - 1 - ty;

		FVector2D p = startFrom;

		for (int i = 0; i < cols; ++i)
		{
			// Screen destination position
			const int ix = static_cast<int>(std::lround(p.x));
			const int iy = static_cast<int>(std::lround(p.y));

			// Normalized u
			float u = (static_cast<float>(i) + 0.5f) * invCols;
			u = std::min(u, 1.0f - std::numeric_limits<float>::epsilon());
			int tx = static_cast<int>(u * static_cast<float>(texW));
			tx = std::clamp(tx, 0, static_cast<int>(texW) - 1);

			if (IsBounded(ix, iy))
			{
				const auto texel = texture->GetPixel(static_cast<uint32_t>(tx),
					static_cast<uint32_t>(ty));
				m_pImageBuffer->WriteAt(iy, ix, texel);
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
