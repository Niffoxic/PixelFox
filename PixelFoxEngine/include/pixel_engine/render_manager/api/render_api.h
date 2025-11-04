// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/*
 *  -----------------------------------------------------------------------------
 *  Project   : PixelFox (WMG Warwick - Module 1)
 *  Author    : Niffoxic (a.k.a Harsh Dubey)
 *  License   : MIT
 *  -----------------------------------------------------------------------------
 */

#pragma once

#include "PixelFoxEngineAPI.h"

#include <sal.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <windows.h>

#include "pixel_engine/utilities/clock/clock.h"
#include "pixel_engine/render_manager/components/camera/camera.h"

#include "raster/raster.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace pixel_engine
{
	typedef struct _CONSTRUCT_RENDER_API_DESC
	{
		_In_ HANDLE StartEvent;
		_In_ HANDLE ExitEvent;

	} CONSTRUCT_RENDER_API_DESC;

	typedef struct _INIT_RENDER_API_DESC
	{
		_In_ int	   Width;
		_In_ int	   Height;
		_In_ HWND	   WindowsHandle;
		_In_ BOOL      FullScreen;
		_In_ Camera2D* Camera;
	} PFE_API INIT_RENDER_API_DESC;

	class PFE_API PERenderAPI
	{
	public:
		 PERenderAPI(_In_ const CONSTRUCT_RENDER_API_DESC* desc);
		~PERenderAPI();

		//~ no move or copy
		PERenderAPI(_In_ const PERenderAPI&) = delete;
		PERenderAPI(_Inout_ PERenderAPI&&)   = delete;

		PERenderAPI& operator=(_In_ const PERenderAPI&) = delete;
		PERenderAPI& operator=(_Inout_ PERenderAPI&&)   = delete;

		bool Init(_In_ const INIT_RENDER_API_DESC* desc);
		
		static DWORD WINAPI RenderThread(_In_ LPVOID ctx)
		{
			return static_cast<PERenderAPI*>(ctx)->Execute();
		}

		DWORD Execute();

		// Must be called from whoever wanna wait for preset
		void WaitForPresent();

	private:
		void CleanFrame();
		void WriteFrame();
		void PresentFrame();

		//~ DirectX Creation
		_NODISCARD _Check_return_
		bool InitializeDirectX			 (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreateDeviceAndDeviceContext(_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreateSwapChain			 (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreateRTV					 (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreateVertexShader			 (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreatePixelShader			 (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool CreateViewport				 (_In_ const INIT_RENDER_API_DESC* desc);

		//~ RenderAPI Core creation
		_NODISCARD _Check_return_
		bool InitializeRenderAPI  (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool InitializeRaster2D   (_In_ const INIT_RENDER_API_DESC* desc);
		_NODISCARD _Check_return_
		bool InitializeRenderQueue(_In_ const INIT_RENDER_API_DESC* desc);

	private:
		//~ Core
		Camera2D*  m_pCamera{ nullptr };
		
		std::unique_ptr<PERaster2D>  m_pRaster2D { nullptr };

		//~ Thread Members
		HANDLE m_handleStartEvent      { nullptr }; // render manager will own it
		HANDLE m_handleExitEvent       { nullptr }; // render manager will signal it
		
		HANDLE m_handlePresentEvent    { nullptr }; // owned by render api so that I can signal if its ready to present
		HANDLE m_handlePresentDoneEvent{ nullptr };

		//~ Render Members
		D3D11_VIEWPORT m_Viewport	   {};
		size_t		   m_PaddedDataSize{ 0u };

		Microsoft::WRL::ComPtr<ID3D11Device>			 m_pDevice		  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		 m_pDeviceContext { nullptr };
		Microsoft::WRL::ComPtr<IDXGISwapChain>			 m_pSwapchain	  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	 m_pRTV			  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11Buffer>			 m_pCpuImageBuffer{ nullptr };
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSRV			  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		 m_pPixelShader	  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11VertexShader>		 m_pVertexShader  { nullptr };
	};
} // pixel_engine
