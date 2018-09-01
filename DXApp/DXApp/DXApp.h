#pragma once
#include <stdio.h>
#include <windows.h>
#include <string>
#include <d3d11.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <tchar.h>
// Microsoft::WRL::ComPtr
#include <wrl.h>

#pragma comment(lib, "d3dcompiler.lib")
#include <DirectXMath.h>
#include "DXUtil.h"


using namespace DirectX;

class DXApp
{
public:
	DXApp(HINSTANCE hInstance);
	virtual ~DXApp(void);

	// main application loop
	int Run();

	// framework api
	virtual bool Initialize();
	virtual void Update(float dt) = 0;
	virtual void Render();
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	struct ConstantBufferMatrix
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX wvp;
	};

	struct SRVPerInstanceData
	{
		XMMATRIX World;
	};

protected:

	HINSTANCE m_hAppInstance;
	HWND m_hAppWnd;
	D3D_DRIVER_TYPE m_DriverType;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pImmediateContext;
	IDXGISwapChain* m_pSwapChain;
	D3D11_VIEWPORT m_Viewport;

	XMMATRIX m_World;
	XMMATRIX m_View;
	XMMATRIX m_Projection;
	XMFLOAT4 m_MeshColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);

	std::string m_AppTitle;
	DWORD m_WndStyle;

	ID3D11RenderTargetView*    m_pRenderTargetView;
	ID3D11Texture2D*           m_pDepthStencil;
	ID3D11DepthStencilView*    m_pDepthStencilView;
	ID3D11DepthStencilState*   m_pDepthStencilState;
	ID3DUserDefinedAnnotation* m_pUserAnnotation = nullptr;
	ID3D11VertexShader* m_VertexShader;
	ID3D11VertexShader* m_InstancingVertexShader;
	ID3D11PixelShader*  m_PixelShader;
	ID3D11Buffer* m_PcBuffer;
	ID3D11Buffer* m_TransformStructuredBuffer;
	ID3D11BlendState*         m_BlendState;
	ID3D11RasterizerState*    m_RasterizerState;
	ID3D11ShaderResourceView* m_TransformSRV;
	const uint32_t m_InstanceMAX = 32;


protected:
	HRESULT InitWindow();
	HRESULT InitDevice();
	virtual HRESULT InitApp();
	virtual HRESULT SetupTransformSRV();
	virtual void InitDirect3DInternal();

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
};

