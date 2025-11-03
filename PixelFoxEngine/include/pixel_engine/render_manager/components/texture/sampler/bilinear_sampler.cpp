#include "pch.h"
#include "bilinear_sampler.h"

#include <cmath>
#include <algorithm>
#include <limits>
#include <cassert>

_Use_decl_annotations_
std::unique_ptr<pixel_engine::Texture> 
pixel_engine::BilinearSampler::GetSampledImage(
    const Texture * rawImage,
    int tileSize,
    const FVector2D & scale,
    EdgeMode edge) const
{
    if (!rawImage || tileSize <= 0) return nullptr;

    const int sx = std::max(1,
        static_cast<int>(std::lround(scale.x * tileSize)));
    const int sy = std::max(1,
        static_cast<int>(std::lround(scale.y * tileSize)));
    
    return SampleToSize(
        rawImage,
        static_cast<uint32_t>(sx),
        static_cast<uint32_t>(sy),
        edge);;
}

_Use_decl_annotations_
std::unique_ptr<pixel_engine::Texture>
pixel_engine::BilinearSampler::SampleToSize(
    const Texture* rawImage,
    uint32_t outWidth,
    uint32_t outHeight,
    EdgeMode edge) const
{
    if (!rawImage || outWidth == 0 || outHeight == 0) return nullptr;

    return SampleRegionToSize(
        rawImage, 0, 0,
        rawImage->GetWidth(),
        rawImage->GetHeight(),
        outWidth,
        outHeight,
        edge);
}

_Use_decl_annotations_
std::unique_ptr<pixel_engine::Texture> 
pixel_engine::BilinearSampler::SampleRegionToSize(
    const Texture* rawImage,
    uint32_t srcX,
    uint32_t srcY,
    uint32_t srcW,
    uint32_t srcH,
    uint32_t outWidth,
    uint32_t outHeight,
    EdgeMode edge) const
{
    if (!rawImage || outWidth == 0 ||
        outHeight == 0 || srcW == 0 ||
        srcH == 0)
        return nullptr;

    // Source info
    const auto fmt           = rawImage->GetFormat();
    const auto cs            = rawImage->GetColorSpace();
    const auto origin        = rawImage->GetOrigin();
    const uint32_t bpp       = rawImage->BytesPerPixel();
    const uint32_t srcStride = rawImage->GetRowStride();
    const uint8_t* srcBase   = rawImage->Data().data;

    // Prepare destination buffer
    const uint32_t dstStride = outWidth * bpp;
    fox::vector<uint8_t> dstBytes;
    dstBytes.resize(static_cast<size_t>(dstStride) * outHeight, 0u);

    // Normalized sampling map output pixel centers to source pixel space
    const float scaleX = (srcW > 1) ? (static_cast<float>(srcW) / static_cast<float>(outWidth)) : 0.0f;
    const float scaleY = (srcH > 1) ? (static_cast<float>(srcH) / static_cast<float>(outHeight)) : 0.0f;

    for (uint32_t y = 0; y < outHeight; ++y)
    {
        // Source y in region (center-of-pixel mapping)
        float syf = (static_cast<float>(y) + 0.5f) * scaleY - 0.5f;
        // Neighbor coords
        int y0 = static_cast<int>(std::floor(syf));
        int y1 = y0 + 1;
        float fy = syf - static_cast<float>(y0);

        // Addressing relative to the region
        const int relY0 = AddressIndex(y0, static_cast<int>(srcH), edge);
        const int relY1 = AddressIndex(y1, static_cast<int>(srcH), edge);

        // Convert to absolute logical y
        const int absY0 = static_cast<int>(srcY) + relY0;
        const int absY1 = static_cast<int>(srcY) + relY1;

        // Convert to memory rows per origin
        const int memY0 = MapYToMemory(absY0, rawImage->GetHeight(), origin);
        const int memY1 = MapYToMemory(absY1, rawImage->GetHeight(), origin);

        for (uint32_t x = 0; x < outWidth; ++x)
        {
            float sxf = (static_cast<float>(x) + 0.5f) * scaleX - 0.5f;
            int x0 = static_cast<int>(std::floor(sxf));
            int x1 = x0 + 1;
            float fx = sxf - static_cast<float>(x0);

            const int relX0 = AddressIndex(x0, static_cast<int>(srcW), edge);
            const int relX1 = AddressIndex(x1, static_cast<int>(srcW), edge);

            const int absX0 = static_cast<int>(srcX) + relX0;
            const int absX1 = static_cast<int>(srcX) + relX1;

            // Sample 4 neighbors
            uint8_t r00, g00, b00, a00;
            uint8_t r10, g10, b10, a10;
            uint8_t r01, g01, b01, a01;
            uint8_t r11, g11, b11, a11;

            FetchPixel(srcBase, srcStride, bpp, absX0, memY0, fmt, r00, g00, b00, a00);
            FetchPixel(srcBase, srcStride, bpp, absX1, memY0, fmt, r10, g10, b10, a10);
            FetchPixel(srcBase, srcStride, bpp, absX0, memY1, fmt, r01, g01, b01, a01);
            FetchPixel(srcBase, srcStride, bpp, absX1, memY1, fmt, r11, g11, b11, a11);

            // Horizontal lerp
            const uint8_t r0 = Lerp8(r00, r10, fx);
            const uint8_t g0 = Lerp8(g00, g10, fx);
            const uint8_t b0 = Lerp8(b00, b10, fx);
            const uint8_t a0 = Lerp8(a00, a10, fx);

            const uint8_t r1 = Lerp8(r01, r11, fx);
            const uint8_t g1 = Lerp8(g01, g11, fx);
            const uint8_t b1 = Lerp8(b01, b11, fx);
            const uint8_t a1 = Lerp8(a01, a11, fx);

            // Vertical lerp
            const uint8_t r = Lerp8(r0, r1, fy);
            const uint8_t g = Lerp8(g0, g1, fy);
            const uint8_t b = Lerp8(b0, b1, fy);
            const uint8_t a = Lerp8(a0, a1, fy);

            // Store
            const size_t dstOff = static_cast<size_t>(y) * static_cast<size_t>(dstStride)
                + static_cast<size_t>(x) * static_cast<size_t>(bpp);
            switch (fmt)
            {
            case TextureFormat::R8:
                dstBytes[dstOff + 0] = r; break;
            case TextureFormat::RG8:
                dstBytes[dstOff + 0] = r;
                dstBytes[dstOff + 1] = g;
                break;
            case TextureFormat::RGB8:
                dstBytes[dstOff + 0] = r;
                dstBytes[dstOff + 1] = g;
                dstBytes[dstOff + 2] = b;
                break;
            case TextureFormat::RGBA8:
                dstBytes[dstOff + 0] = r;
                dstBytes[dstOff + 1] = g;
                dstBytes[dstOff + 2] = b;
                dstBytes[dstOff + 3] = a;
                break;
            default:
                break;
            }
        }
    }

    return std::make_unique<Texture>(
        rawImage->GetFilePath(),
        outWidth, outHeight,
        fmt,
        cs,
        std::move(dstBytes),
        dstStride,
        Origin::TopLeft,
        false
    );
}

int pixel_engine::BilinearSampler::ClampIndex(int i, int n) const noexcept
{
    if (i < 0)  return 0;
    if (i >= n) return n - 1;
    return i;
}

int pixel_engine::BilinearSampler::WrapIndex(int i, int n) const noexcept
{
    if (n <= 0) return 0;
    int m = i % n;
    return (m < 0) ? (m + n) : m;
}

int pixel_engine::BilinearSampler::MirrorIndex(int i, int n) const noexcept
{
    if (n <= 1) return 0;
    const int period = 2 * n - 2;
    int m = i % period;
    if (m < 0) m += period;
    return (m < n) ? m : (period - m);
}

int pixel_engine::BilinearSampler::AddressIndex(int i,
    int n, EdgeMode mode) const noexcept
{
    switch (mode)
    {
    case EdgeMode::Wrap:   return WrapIndex(i, n);
    case EdgeMode::Mirror: return MirrorIndex(i, n);
    default:               return ClampIndex(i, n);
    }
}

int pixel_engine::BilinearSampler::MapYToMemory(
    int yLogical, uint32_t texHeight, Origin origin) const noexcept
{
    if (origin == Origin::TopLeft) return yLogical;
    return static_cast<int>(texHeight) - 1 - yLogical;
}

uint8_t pixel_engine::BilinearSampler::FetchChan(const uint8_t* base,
    uint32_t rowStride, uint32_t bpp, int x, int y) const noexcept
{
    const size_t off =  static_cast<size_t>(y)          *
                        static_cast<size_t>(rowStride)  +
                        static_cast<size_t>(x)          *
                        static_cast<size_t>(bpp);
    return base[off];
}

void pixel_engine::BilinearSampler::FetchPixel(const uint8_t* base,
    uint32_t rowStride, uint32_t bpp, int x, int y,
    TextureFormat fmt, uint8_t& r, uint8_t& g, uint8_t& b,
    uint8_t& a) const noexcept
{
    const size_t off =  static_cast<size_t>(y)          *
                        static_cast<size_t>(rowStride)  +
                        static_cast<size_t>(x)          *
                        static_cast<size_t>(bpp);

    const uint8_t* p = base + off;

    switch (fmt)
    {
    case TextureFormat::R8:
        r = g = b = p[0];
        a = 255; 
        break;
    case TextureFormat::RG8:
        r = p[0];
        g = p[1];
        b = 0;
        a = 255; 
        break;
    case TextureFormat::RGB8:
        r = p[0];
        g = p[1];
        b = p[2]; 
        a = 255; 
        break;
    case TextureFormat::RGBA8:
        r = p[0];
        g = p[1];
        b = p[2];
        a = p[3]; 
        break;
    default:
        r = g = b = 0; a = 255; 
        break;
    }
}

void pixel_engine::BilinearSampler::StorePixel(uint8_t* base,
    uint32_t rowStride, uint32_t bpp, int x, int y,
    TextureFormat fmt, uint8_t r, uint8_t g, uint8_t b,
    uint8_t a) const noexcept
{
    const size_t off =  static_cast<size_t>(y)          *
                        static_cast<size_t>(rowStride)  +
                        static_cast<size_t>(x)          *
                        static_cast<size_t>(bpp);
    uint8_t* p = base + off;

    switch (fmt)
    {
    case TextureFormat::R8:
        p[0] = r;
        break;
    case TextureFormat::RG8:
        p[0] = r;
        p[1] = g;
        break;
    case TextureFormat::RGB8:
        p[0] = r;
        p[1] = g;
        p[2] = b;
        break;
    case TextureFormat::RGBA8:
        p[0] = r;
        p[1] = g;
        p[2] = b;
        p[3] = a;
        break;
    default:
        break;
    }
}

uint8_t pixel_engine::BilinearSampler::Lerp8(uint8_t a,
    uint8_t b, float t) const noexcept
{
    const float af = static_cast<float>(a);
    const float bf = static_cast<float>(b);
    
    int v = static_cast<int>(af + (bf - af) * t + 0.5f);
    
    if (v < 0) v = 0; else if (v > 255) v = 255;
    
    return static_cast<uint8_t>(v);
}
