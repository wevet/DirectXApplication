#include "Application.h"

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

	m_FBXRenderer = new FBXRenderer;
	HRESULT hr = S_OK;
	hr = m_FBXRenderer->LoadFBX("Assets/FBX/Miku_default/Miku_default.fbx", m_pDevice);

	if (FAILED(hr))
	{
		OutputDebugString("\n Failed to FBXRenderer \n");
	}
	else
	{
		OutputDebugString("\n Success to FBXRenderer \n");
	}
}
