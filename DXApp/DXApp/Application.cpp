#include "Application.h"
#include <d3dcompiler.h>


using namespace Prototype;

Application::Application(HINSTANCE hInstance) : DXApp(hInstance)
{
}


Application::~Application()
{
	//
}

bool Application::Initialize()
{
	if (!DXApp::Initialize())
	{
		return false;
	}

	InitDirect3DInternal();
	return true;

}

void Application::Update(float dt)
{
	m_pos += (m_pos * dt);
}

void Application::Render(float dt)
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::Black);

	// draw spritebatch
	spriteBatch->Begin();
	spriteBatch->Draw(m_pTexture.Get(), m_pos);
	spriteBatch->End();
	HR(m_pSwapChain->Present(0, 0));
}

void Application::InitDirect3DInternal()
{
	spriteBatch.reset(new SpriteBatch(m_pImmediateContext));
	HR(CreateDDSTextureFromFile(m_pDevice, L"Assets/Sample.dds", nullptr, &m_pTexture));
	DXApp::InitDirect3DInternal();

}

HRESULT Application::InitApp()
{

	HRESULT hr = S_OK;
	m_FBXRenderer = new FBXRenderer;
	hr = m_FBXRenderer->LoadFBX("Assets/FBX/Miku_default/Miku_default.fbx", m_pDevice);

	if (FAILED(hr))
	{
		OutputDebugString("\n Failed to FBXRenderer \n");
		return hr;
	}
	else
	{
		OutputDebugString("\n Success to FBXRenderer \n");
	}
	UINT compileFlags = 0;
#if defined(DEBUG)
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// compile vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"Shaders/simpleRenderVS.hlsl", "vsMain", "vs_4_0", &pVSBlob);
	if (FAILED(hr)) 
	{
		OutputDebugString("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.");
		return hr;
	}

	// create vertex shader
	hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_VertexShader);
	if (FAILED(hr)) 
	{
		Memory::SafeRelease(pVSBlob);
		return hr;
	}
	Memory::SafeRelease(pVSBlob);

	// compile instancing vertex shader
	hr = CompileShaderFromFile(L"simpleRenderInstancingVS.hlsl", "vsMain", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		OutputDebugString("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.");
		return hr;
	}

	// create instancing vertex shader
	hr = m_pDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_InstancingVertexShader);
	if (FAILED(hr))
	{
		Memory::SafeRelease(pVSBlob);
		return hr;
	}

	// input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	UINT numElements = ARRAYSIZE(layout);

	// todo
	hr = m_FBXRenderer->CreateInputLayout(m_pDevice, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), layout, numElements);
	Memory::SafeRelease(pVSBlob);
	if (FAILED(hr))
	{
		return hr;
	}

	// compile pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"simpleRenderPS.hlsl", "psMain", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		OutputDebugString("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.");
		return hr;
	}

	return S_OK;
}
