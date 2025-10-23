#pragma once
#include "PixelFoxEngineAPI.h"
#include "pixel_engine/core/interface/interface_manager.h"
#include "core/unordered_map.h"
#include "pixel_engine/core/event/event_queue.h"
#include "pixel_engine/window_manager/windows_manager.h"

#include <wrl/client.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace pixel_engine
{
	class PFE_API PERenderManager final: public IManager
	{
	public:
		 PERenderManager(_In_ PEWindowsManager* windows);
		~PERenderManager() = default;

		//~ interface implementation
		_NODISCARD _Check_return_ bool OnInit   () override;
		_NODISCARD _Check_return_ bool OnRelease() override;

		_NODISCARD _Check_return_ __forceinline
		std::string GetManagerName() const override { return "RenderManager"; }

		void OnLoopStart(_In_ float deltaTime) override;
		void OnLoopEnd  () override;

	private:
		bool CreateDeviceAndDeviceContext();
		bool CreateSwapChain			 ();
		bool CreateRTV					 ();
		bool CreateVertexShader			 ();
		bool CreatePixelShader			 ();
		bool CreateViewport				 ();

		void SubscribeToEvents  ();
		void UnSubscribeToEvents();

	private:
		PEWindowsManager*	  m_pWindowsManager{ nullptr };
		fox::vector<SubToken> m_eventTokens{};

		Microsoft::WRL::ComPtr<ID3D11Device>			 m_pDevice       { nullptr };
		Microsoft::WRL::ComPtr<ID3D11DeviceContext>		 m_pDeviceContext{ nullptr };
		Microsoft::WRL::ComPtr<IDXGISwapChain>			 m_pSwapchain    { nullptr };
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	 m_pRTV          { nullptr };
		Microsoft::WRL::ComPtr<ID3D11Buffer>			 m_pBackBuffer   { nullptr };
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSRV		     { nullptr };
		Microsoft::WRL::ComPtr<ID3D11PixelShader>		 m_pPixelShader  { nullptr };
		Microsoft::WRL::ComPtr<ID3D11VertexShader>		 m_pVertexShader { nullptr };
	};
}
