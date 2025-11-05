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
#include <filesystem>

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
    std::wstring wFileName = std::filesystem::path(path).wstring();
    THROW_WIN_IF_FAILS(stream->InitializeFromFilename(wFileName.c_str(), GENERIC_READ));

    ComPtr<IWICBitmapDecoder> decoder;
    THROW_WIN_IF_FAILS(factory->CreateDecoderFromStream(
        stream.Get(), nullptr, WICDecodeMetadataCacheOnDemand, &decoder));

    ComPtr<IWICBitmapFrameDecode> frame;
    THROW_WIN_IF_FAILS(decoder->GetFrame(0, &frame));

    // Dimensions
    UINT width = 0, height = 0;
    THROW_WIN_IF_FAILS(frame->GetSize(&width, &height));

    GUID targetFormat = GUID_WICPixelFormat32bppRGBA;
    ComPtr<IWICFormatConverter> converter;
    THROW_WIN_IF_FAILS(factory->CreateFormatConverter(&converter));

    BOOL can = FALSE;
    hr = converter->CanConvert(
        [&]() -> WICPixelFormatGUID {
            WICPixelFormatGUID pf{};
            frame->GetPixelFormat(&pf);
            return pf;
        }(),
            targetFormat,
            &can
            );
    THROW_WIN_IF_FAILS(hr);

    ComPtr<IWICBitmapSource> src;
    if (can)
    {
        THROW_WIN_IF_FAILS(converter->Initialize(
            frame.Get(),
            targetFormat,
            WICBitmapDitherTypeNone,
            nullptr, 0.0f,
            WICBitmapPaletteTypeCustom));
        src = converter;
    }
    else
    {
        targetFormat = GUID_WICPixelFormat32bppBGRA;
        BOOL canBGRA = FALSE;
        THROW_WIN_IF_FAILS(converter->CanConvert(
            [&]() -> WICPixelFormatGUID {
                WICPixelFormatGUID pf{};
                frame->GetPixelFormat(&pf);
                return pf;
            }(),
                targetFormat,
                &canBGRA
                ));
        if (!canBGRA)
        {
            logger::error("WIC cannot convert '{}' to 32-bit RGBA/BGRA", path);
            return nullptr;
        }

        THROW_WIN_IF_FAILS(converter->Initialize(
            frame.Get(),
            targetFormat,
            WICBitmapDitherTypeNone,
            nullptr, 0.0f,
            WICBitmapPaletteTypeCustom));
        src = converter;
    }

    const UINT channels = 4u;
    const UINT tightStride = width * channels;
    const size_t bufferSize = static_cast<size_t>(tightStride) * height;

    std::vector<uint8_t> data(bufferSize);
    THROW_WIN_IF_FAILS(src->CopyPixels(
        nullptr,
        tightStride,
        static_cast<UINT>(data.size()),
        data.data()));

    if (targetFormat == GUID_WICPixelFormat32bppBGRA)
    {
        uint8_t* p = data.data();
        const size_t pxCount = static_cast<size_t>(width) * height;
        for (size_t i = 0; i < pxCount; ++i)
        {
            const size_t base = i * 4;
            std::swap(p[base + 0], p[base + 2]);
        }
    }

    fox::vector<uint8_t> texBytes;
    texBytes.resize(data.size());
    if (!data.empty())
        memcpy(texBytes.data(), data.data(), data.size());

    logger::success("Loaded Image has {} width and {} height", width, height);

    return std::make_unique<pixel_engine::Texture>(
        path,
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height),
        TextureFormat::RGBA8,    
        ColorSpace::sRGB,         
        std::move(texBytes),
        tightStride,               
        Origin::TopLeft,
        false               
    );
}
