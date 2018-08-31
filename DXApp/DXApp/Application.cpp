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

	ID3DBlob* pVSBlob = NULL;
	//hr = CompileShaderFromFile(L"simpleRenderVS.hlsl", "vs_main", "vs_4_0", &pVSBlob);

	return S_OK;
}
