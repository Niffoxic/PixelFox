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
#include "png_loader.h"

#include <windows.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <wincodecsdk.h>

#pragma comment(lib, "WindowsCodecs.lib")

#include "pixel_engine/render_manager/components/texture/texture.h"
#include "pixel_engine/exceptions/win_exception.h"

_Use_decl_annotations_
std::unique_ptr<pixel_engine::Texture> 
pixel_engine::PNGLoader::LoadTexture(const std::string& path)
{
    using Microsoft::WRL::ComPtr;

    ComPtr<IWICImagingFactory> factory;
    HRESULT hr = ::CoCreateInstance(
        CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&factory)
    );
    THROW_WIN_IF_FAILS(hr);

    ComPtr<IWICStream> stream;
    THROW_WIN_IF_FAILS(factory->CreateStream(&stream));

    std::wstring wFileName(path.begin(), path.end());
    THROW_WIN_IF_FAILS(stream->InitializeFromFilename(wFileName.c_str(), GENERIC_READ));

    ComPtr<IWICBitmapDecoder> decoder;
    THROW_WIN_IF_FAILS(factory->CreateDecoderFromStream(
        stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));

    ComPtr<IWICBitmapFrameDecode> frame;
    THROW_WIN_IF_FAILS(decoder->GetFrame(0, &frame));

    unsigned width = 0, height = 0;
    THROW_WIN_IF_FAILS(frame->GetSize(&width, &height));

    WICPixelFormatGUID pixelFormat = {};
    THROW_WIN_IF_FAILS(frame->GetPixelFormat(&pixelFormat));

    unsigned channels = 0;
    int isRGB = 0;

    if (pixelFormat == GUID_WICPixelFormat24bppBGR)  { channels = 3; isRGB = 0; }
    if (pixelFormat == GUID_WICPixelFormat32bppBGRA) { channels = 4; isRGB = 0; }
    if (pixelFormat == GUID_WICPixelFormat24bppRGB)  { channels = 3; isRGB = 1; }
    if (pixelFormat == GUID_WICPixelFormat32bppRGBA) { channels = 4; isRGB = 1; }

    if (channels == 0)
    {
        return nullptr;
    }

    std::vector<unsigned char> data;
    data.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * channels);

    const unsigned tightStride  = width * channels;
    const unsigned paddedStride = (tightStride + 3u) & ~3u;

    if (paddedStride == tightStride)
    {
        THROW_WIN_IF_FAILS(frame->CopyPixels(
            nullptr, tightStride,
            static_cast<UINT>(data.size()),
            data.data()));
    }
    else
    {
        std::vector<unsigned char> strideBuffer;
        strideBuffer.resize(static_cast<size_t>(paddedStride) * height);

        THROW_WIN_IF_FAILS(frame->CopyPixels(
            nullptr, paddedStride,
            static_cast<UINT>(strideBuffer.size()),
            strideBuffer.data()));

        for (unsigned int y = 0; y < height; ++y)
        {
            const unsigned char* src = strideBuffer.data() + static_cast<size_t>(y) * paddedStride;
            unsigned char* dst = data.data() + static_cast<size_t>(y) * tightStride;
            memcpy(dst, src, tightStride);
        }
    }

    if (isRGB == 0)
    {
        for (size_t i = 0, px = static_cast<size_t>(width) * height; i < px; ++i)
        {
            const size_t base = i * channels;
            std::swap(data[base + 0], data[base + 2]);
        }
    }

    TextureFormat fmt = (channels == 3) ? TextureFormat::RGB8 : TextureFormat::RGBA8;

    fox::vector<uint8_t> texBytes;
    texBytes.resize(data.size());
    if (!data.empty())
        memcpy(texBytes.data(), data.data(), data.size());

    // Build Texture
    const uint32_t w = static_cast<uint32_t>(width);
    const uint32_t h = static_cast<uint32_t>(height);
    const uint32_t strideBytes = static_cast<uint32_t>(tightStride);

    return std::make_unique<pixel_engine::Texture>(
        path,
        w, h,
        fmt,
        ColorSpace::sRGB,
        std::move(texBytes),
        strideBytes,
        Origin::TopLeft,
        false
    );
}
