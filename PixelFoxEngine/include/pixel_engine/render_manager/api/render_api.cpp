#include "pch.h"
#include "render_api.h"

#include "pixel_engine/utilities/logger/logger.h" 
#include "core/vector.h"

// TODO: Make logger thread safe!

pixel_engine::PERenderAPI::PERenderAPI(const CONSTRUCT_RENDER_API_DESC* desc)
{
    m_handleStartEvent = desc->StartEvent;
    m_handleExitEvent  = desc->ExitEvent;

    m_handlePresentEvent = CreateEvent(
        nullptr,
        TRUE,
        FALSE,
        nullptr
    );

    m_handlePresentDoneEvent = CreateEvent(
        nullptr,
        TRUE,
        FALSE,
        nullptr
    );
}

pixel_engine::PERenderAPI::~PERenderAPI()
{
    if (m_handlePresentEvent)     CloseHandle(m_handlePresentEvent);
    if (m_handlePresentDoneEvent) CloseHandle(m_handlePresentDoneEvent);
}

bool pixel_engine::PERenderAPI::Init(const INIT_RENDER_API_DESC* desc)
{
    if (not CreateDeviceAndDeviceContext(desc)) return false;
    if (not CreateSwapChain(desc))              return false;
    if (not CreateRTV(desc))                    return false;
    if (not CreateVertexShader(desc))           return false;
    if (not CreatePixelShader(desc))            return false;
    if (not CreateViewport(desc))               return false;

    return true;
}

DWORD pixel_engine::PERenderAPI::Execute()
{
    WaitForSingleObject(m_handleStartEvent, INFINITE);
    
    while (true)
    {
        if (WaitForSingleObject(m_handleExitEvent, 0) == WAIT_OBJECT_0)
        {
            return 0u;
        }

        //~ Prepare Images

        //~ Wait for signal when to present
        WaitForSingleObject(m_handlePresentEvent, INFINITE);
        Present();
        SetEvent(m_handlePresentDoneEvent); // present done
    }
    return 0;
}

void pixel_engine::PERenderAPI::WaitForPresent()
{
    ResetEvent(m_handlePresentDoneEvent);
    SetEvent(m_handlePresentEvent);
    WaitForSingleObject(m_handlePresentDoneEvent, INFINITE);
}

void pixel_engine::PERenderAPI::Present()
{
    const float clear[4] = { 1.f, 1.0f, 1.0f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRTV.Get(), clear);

    TestImageUpdate();

    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);
    m_pDeviceContext->Draw(3, 0);
    m_pSwapchain->Present(1u, 0u);
}

bool pixel_engine::PERenderAPI::CreateDeviceAndDeviceContext(const INIT_RENDER_API_DESC* desc)
{
    UINT createFlags = 0;
#if defined (DEBUG) || defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    D3D_FEATURE_LEVEL availableLevel = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &m_pDevice,
        &availableLevel,
        &m_pDeviceContext
    );

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Create Device!");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Device and Context!");

    return true;
}

bool pixel_engine::PERenderAPI::CreateSwapChain(const INIT_RENDER_API_DESC* desc)
{
    Microsoft::WRL::ComPtr<IDXGIDevice> m_pDxgiDevice;
    HRESULT hr = m_pDevice.As(&m_pDxgiDevice);

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Translate Device as a dxgi device!");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> m_pAdapter;
    hr = m_pDxgiDevice->GetAdapter(&m_pAdapter);

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Get Adapter whcih used to create device and context!");
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory> m_pFactory;
    hr = m_pAdapter->GetParent(__uuidof(IDXGIFactory),
        reinterpret_cast<void**>(m_pFactory.GetAddressOf()));

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Get factory whcih used to create adapter!");
        return false;
    }

    //~ Creates Swapchain
    DXGI_SWAP_CHAIN_DESC sdesc{};
    sdesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sdesc.BufferCount                        = 1u;
    sdesc.OutputWindow                       = desc->WindowsHandle;
    sdesc.SampleDesc.Count                   = 1u;
    sdesc.SampleDesc.Quality                 = 0u;
    sdesc.Windowed                           = FALSE;
    sdesc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;
    sdesc.Flags                              = 0u;
    sdesc.BufferDesc.Width                   = desc->Width;
    sdesc.BufferDesc.Height                  = desc->Height;
    sdesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    sdesc.BufferDesc.RefreshRate.Numerator   = 60u;
    sdesc.BufferDesc.RefreshRate.Denominator = 1u;

    hr = m_pFactory->CreateSwapChain(m_pDevice.Get(),
        &sdesc, m_pSwapchain.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to Create Swapchain");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Swapchain!");

    if (desc->FullScreen) m_pSwapchain->SetFullscreenState(TRUE, nullptr);
    else                  m_pSwapchain->SetFullscreenState(FALSE,  nullptr);
    
    return true;
}

bool pixel_engine::PERenderAPI::CreateRTV(const INIT_RENDER_API_DESC* desc)
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_texture{ nullptr };
    HRESULT hr = m_pSwapchain->GetBuffer(0,
        __uuidof(ID3D11Texture2D), reinterpret_cast<void**>
        (back_texture.GetAddressOf()));

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to acquire back buffer texture");
        return false;
    }

    hr = m_pDevice->CreateRenderTargetView(
        back_texture.Get(),
        nullptr,
        m_pRTV.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create RTV");
        return false;
    }

    //~ Set states once since only 1 texture will be rendering pixels
    m_pDeviceContext->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);
    m_pDeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    m_pDeviceContext->OMSetDepthStencilState(nullptr, 0);
    m_pDeviceContext->RSSetState(nullptr);
    m_pDeviceContext->IASetInputLayout(nullptr);
    m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    return true;
}

bool pixel_engine::PERenderAPI::CreateVertexShader(const INIT_RENDER_API_DESC* desc)
{
    //~ Fullscreen quad
    static const char* kVS = R"(
        struct VSOut {
            float4 pos : SV_Position;
        };
        VSOut VS(uint vertexID : SV_VertexId)
        {
            VSOut o;
            float2 tex = float2((vertexID << 1) & 2, vertexID & 2);
            o.pos = float4(tex * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
            return o;
        }
    )";

    Microsoft::WRL::ComPtr<ID3DBlob> vsblob, err;

    HRESULT hr = D3DCompile(kVS, strlen(kVS),
        nullptr, nullptr, nullptr, "VS", "vs_5_0",
        0, 0,
        vsblob.GetAddressOf(), err.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create vertex blob");
        return false;
    }

    hr = m_pDevice->CreateVertexShader(vsblob->GetBufferPointer(),
        vsblob->GetBufferSize(), nullptr, m_pVertexShader.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create vertex shader");
        return false;
    }
    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Vertex Shader!");

    m_pDeviceContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0u);

    return true;
}

bool pixel_engine::PERenderAPI::CreatePixelShader(const INIT_RENDER_API_DESC* desc)
{
    const char* psSrc = R"(
    ByteAddressBuffer buf : register(t0);
    struct VSOut { float4 pos : SV_Position; };
    float4 PS(VSOut i) : SV_Target
    {
        int px = clamp(int(i.pos.x), 0, WIDTH-1);
        int py = clamp(int(i.pos.y), 0, HEIGHT-1);
        uint pixelIndex = py * WIDTH + px;

        uint offset = pixelIndex * 3;
        uint inner = offset & 3;
        uint base  = offset & ~3;

        uint data;
        if (inner == 0)
        {
            data = buf.Load(base) & 0x00FFFFFF;
        }
        else
        {
            uint lo = buf.Load(base);
            uint hi = buf.Load(base + 4);
            data = ((lo >> (inner * 8)) | (hi << ((4 - inner) * 8))) & 0x00FFFFFF;
        }

        float r = ( data        & 0xFF) / 255.0;
        float g = ((data >>  8) & 0xFF) / 255.0;
        float b = ((data >> 16) & 0xFF) / 255.0;
        return float4(r,g,b,1.0);
    }
)";

    std::string wStr = std::to_string(desc->Width);
    std::string hStr = std::to_string(desc->Height);
    D3D_SHADER_MACRO macros[] = 
    {
        { "WIDTH",  wStr.c_str() },
        { "HEIGHT", hStr.c_str() },
        { nullptr, nullptr }
    };

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> psBlob, err;
    HRESULT hr = D3DCompile(
        psSrc, strlen(psSrc),
        nullptr,
        macros,
        nullptr,
        "PS", "ps_5_0",
        flags, 0,
        psBlob.GetAddressOf(),
        err.GetAddressOf()
    );

    if (FAILED(hr))
    {
        if (err)
        {
            // TODO: figure out how to print error message
            logger::error(pixel_engine::logger_config::LogCategory::Render,
                "error message occured");
        }
        else
        {
            logger::error(pixel_engine::logger_config::LogCategory::Render,
                "PS compile failed (no error blob)");
        }
        return false;
    }

    hr = m_pDevice->CreatePixelShader(psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(), nullptr, m_pPixelShader.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create pixel shader");
        return false;
    }

    logger::success(pixel_engine::logger_config::LogCategory::Render,
        "Created Pixel Shader!");

    //~ configure backbuffer as a texture
    const size_t dataSize =
        static_cast<size_t>(desc->Width) *
        static_cast<size_t>(desc->Height) * 3u;

    m_PaddedDataSize = ((dataSize + 3u) / 4u) * 4u;

    D3D11_BUFFER_DESC bdesc{};
    bdesc.ByteWidth = static_cast<UINT>(m_PaddedDataSize);
    bdesc.Usage = D3D11_USAGE_DEFAULT;
    bdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdesc.CPUAccessFlags = 0u;
    bdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    m_pCpuImageBuffer.Reset();
    hr = m_pDevice->CreateBuffer(&bdesc, nullptr, m_pCpuImageBuffer.GetAddressOf());
    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create shader resource on backbuffer");
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srvd{};
    srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    srvd.Format = DXGI_FORMAT_R32_TYPELESS;
    srvd.BufferEx.FirstElement = 0;
    srvd.BufferEx.NumElements = static_cast<UINT>(m_PaddedDataSize / 4u);
    srvd.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;

    m_pSRV.Reset();
    hr = m_pDevice->CreateShaderResourceView(
        m_pCpuImageBuffer.Get(),
        &srvd,
        m_pSRV.GetAddressOf());

    if (FAILED(hr))
    {
        logger::error(
            pixel_engine::logger_config::LogCategory::Render,
            "Failed to create shader resource view outta backbuffer");
        return false;
    }

    //~ Bind PS and SRV
    m_pDeviceContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0u);

    ID3D11ShaderResourceView* srvs[] = { m_pSRV.Get() };
    m_pDeviceContext->PSSetShaderResources(0u, 1u, srvs);

    logger::success(
        pixel_engine::logger_config::LogCategory::Render,
        "Initialized and Configured Pixel Shader!");
    return true;
}

bool pixel_engine::PERenderAPI::CreateViewport(const INIT_RENDER_API_DESC* desc)
{
    m_Viewport.Width = static_cast<FLOAT>(desc->Width);
    m_Viewport.Height = static_cast<FLOAT>(desc->Height);
    m_Viewport.MinDepth = 0.0f;
    m_Viewport.MaxDepth = 1.0f;
    m_Viewport.TopLeftX = 0.0f;
    m_Viewport.TopLeftY = 0.0f;

    m_pDeviceContext->RSSetViewports(1, &m_Viewport);
    m_pDeviceContext->OMSetRenderTargets(1, m_pRTV.GetAddressOf(), nullptr);

    logger::success(
        pixel_engine::logger_config::LogCategory::Render,
        "Initialized and Configured Viewport");

    return true;
}

void pixel_engine::PERenderAPI::TestImageUpdate()
{
    const UINT w = m_Viewport.Width;
    const UINT h = m_Viewport.Height;
    const size_t testDataSize = size_t(w) * size_t(h) * 3u;
    const size_t padded = (testDataSize + 3u) & ~size_t(3);

    fox::vector<unsigned char> cpu(padded, 0);
    for (UINT y = 0; y < h; ++y)
    {
        for (UINT x = 0; x < w; ++x)
        {
            size_t i = (size_t(y) * w + x) * 3u;
            cpu[i + 0] = (unsigned char)(255.0f * (float)x / (float)w);
            cpu[i + 1] = (unsigned char)(255.0f * (float)y / (float)h);
            cpu[i + 2] = 33;
        }
    }
    m_pDeviceContext->UpdateSubresource(m_pCpuImageBuffer.Get(), 0, nullptr, cpu.data(), 0, 0);
}
