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
	virtual void Render(float dt) = 0;
	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	struct ConstantBufferMatrix
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMMATRIX mvp;
	};

protected:

	HINSTANCE m_hAppInstance;
	HWND m_hAppWnd;
	D3D_DRIVER_TYPE m_DriverType;
	D3D_FEATURE_LEVEL m_FeatureLevel;
	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pImmediateContext;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11Texture2D* m_pDepthStencil;
	ID3D11DepthStencilView* m_pDepthStencilView;
	ID3D11DepthStencilState* m_pDepthStencilState;
	D3D11_VIEWPORT m_Viewport;

	XMMATRIX m_pWorld;
	XMMATRIX m_pView;
	XMMATRIX m_pProjection;
	XMFLOAT4 m_pMeshColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	ID3DUserDefinedAnnotation* m_pUserAnnotation = nullptr;

	UINT m_SwapChainCount;
	std::string m_AppTitle;
	DWORD m_WndStyle;
	ID3D11VertexShader* m_VertexShader;
	ID3D11VertexShader* m_InstancingVertexShader;
	ID3D11PixelShader* m_PixelShader;

protected:
	HRESULT InitWindow();
	HRESULT InitDevice();
	virtual HRESULT InitApp();
	virtual void InitDirect3DInternal();

	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
};

