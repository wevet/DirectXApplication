#include "Application.h"


using namespace Prototype;

Application::Application(HINSTANCE hInstance) : DXApp(hInstance)
{
	fbxLoader.reset(new PrototypeTools::FBXLoader());
}


Application::~Application()
{
	//Memory::SafeRelease(m_pTexture);
	fbxLoader.release();
}

bool Application::Initialize()
{
	if (!DXApp::Initialize())
	{
		return false;
	}

	if (!fbxLoader->Initialize(
		m_pDevice, 
		m_pImmediateContext, 
		m_Viewport))
	{
		OutputDebugString("\n Failed to fbx loader class \n");
		return false;
	}

	InitDirect3DInternal();
	return true;

}

void Application::Update(float dt)
{
	//m_pos += (m_pos * dt);
}

void Application::Render(float dt)
{
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::SlateGray);

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
