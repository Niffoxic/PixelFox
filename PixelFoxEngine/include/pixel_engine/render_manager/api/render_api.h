#pragma once

#include "PixelFoxEngineAPI.h"

#include <sal.h>
#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <windows.h>

#include "pixel_engine/utilities/clock/clock.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace pixel_engine
{
	typedef struct _CONSTRUCT_RENDER_API_DESC
	{
		HANDLE StartEvent;
		HANDLE ExitEvent;

	} CONSTRUCT_RENDER_API_DESC;

	typedef struct _INIT_RENDER_API_DESC
	{
		int	 Width;
		int	 Height;
		HWND WindowsHandle;
		BOOL FullScreen;
		GameClock* Clock;
	} PFE_API INIT_RENDER_API_DESC;

	class PFE_API PERenderAPI
	{
	public:
		 PERenderAPI(const CONSTRUCT_RENDER_API_DESC* desc);
		~PERenderAPI();

		//~ no move or copy
		PERenderAPI(_In_ const PERenderAPI&) = delete;
		PERenderAPI(_Inout_ PERenderAPI&&)   = delete;

		PERenderAPI& operator=(_In_ const PERenderAPI&) = delete;
		PERenderAPI& operator=(_Inout_ PERenderAPI&&)   = delete;

		bool Init(const INIT_RENDER_API_DESC* desc);
		
		static DWORD WINAPI RenderThread(LPVOID ctx) 
		{
			return static_cast<PERenderAPI*>(ctx)->Execute();
		}

		DWORD Execute();

		// Must be called from whoever wanna wait for preset
		void WaitForPresent();

	private:
		void CleanFrame();
		void WriteFrame(float deltaTime);
		void PresentFrame();

		bool CreateDeviceAndDeviceContext(const INIT_RENDER_API_DESC* desc);
		bool CreateSwapChain			 (const INIT_RENDER_API_DESC* desc);
		bool CreateRTV					 (const INIT_RENDER_API_DESC* desc);
		bool CreateVertexShader			 (const INIT_RENDER_API_DESC* desc);
		bool CreatePixelShader			 (const INIT_RENDER_API_DESC* desc);
		bool CreateViewport				 (const INIT_RENDER_API_DESC* desc);

		void TestImageUpdate(float deltaTime);

	private:

		//~ Core
		GameClock* m_pClock{ nullptr };

		//~ Thread Members
		HANDLE m_handleStartEvent      { nullptr }; // render manager will own it
		HANDLE m_handleExitEvent       { nullptr }; // render manager will signal it
		
		HANDLE m_handlePresentEvent    { nullptr }; // owned by render api so that I can signal if its ready to present
		HANDLE m_handlePresentDoneEvent{ nullptr };

		//~ Render Members
		D3D11_VIEWPORT		  m_Viewport	  {};
		size_t				  m_PaddedDataSize{ 0u };

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
