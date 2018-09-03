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

void Application::InitDirect3DInternal()
{
	spriteBatch.reset(new SpriteBatch(m_pImmediateContext));
	HR(CreateDDSTextureFromFile(m_pDevice, L"Assets/Sample.dds", nullptr, &m_pTexture));
	DXApp::InitDirect3DInternal();
}

HRESULT Application::InitApp()
{

	HRESULT hr = S_OK;
	const char* fileName = "Assets/FBX/Miku_default/sample.fbx";
	m_FBXRenderer = new FBXRenderer;
	hr = m_FBXRenderer->LoadFBX(fileName, m_pDevice);

	if (FAILED(hr))
	{
		OutputDebugString("\n Failed to FBXRenderer \n");
		return hr;
	}
	else
	{
		OutputDebugString("\n Success to FBXRenderer \n");
	}
	UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG)
	compileFlags |= D3DCOMPILE_DEBUG;
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
	hr = CompileShaderFromFile(L"Shaders/simpleRenderInstancingVS.hlsl", "vsMain", "vs_4_0", &pVSBlob);
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
	hr = CompileShaderFromFile(L"Shaders/simpleRenderPS.hlsl", "psMain", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		OutputDebugString("The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.");
		return hr;
	}
	hr = m_pDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_PixelShader);
	Memory::SafeRelease(pPSBlob);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed PixelShader \n");
		return hr;
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(ConstantBufferMatrix);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = m_pDevice->CreateBuffer(&bd, NULL, &m_PcBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = FALSE;
	m_pDevice->CreateRasterizerState(&rsDesc, &m_RasterizerState);
	m_pImmediateContext->RSSetState(m_RasterizerState);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_pDevice->CreateBlendState(&blendDesc, &m_BlendState);

	// SpriteBatch
	//spriteBatch = new DirectX::SpriteBatch(g_pImmediateContext);
	// SpriteFont
	//spriteFont = new DirectX::SpriteFont(g_pd3dDevice, L"Assets\\Arial.spritefont");

	return hr;
}

HRESULT Application::SetupTransformSRV()
{
	HRESULT hr = S_OK;
	const uint32_t count = m_InstanceMAX;
	const uint32_t stride = static_cast<uint32_t>(sizeof(SRVPerInstanceData));

	// Create StructuredBuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;
	hr = m_pDevice->CreateBuffer(&bd, NULL, &m_TransformStructuredBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	// Create ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements = count;

	hr = m_pDevice->CreateShaderResourceView(m_TransformStructuredBuffer, &srvDesc, &m_TransformSRV);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

void Application::Update(float dt)
{
	m_pos += (m_pos * dt);
}

void Application::Render()
{
	// draw spritebatch
	//spriteBatch->Begin();
	//spriteBatch->Draw(m_pTexture.Get(), m_pos);
	//spriteBatch->End();

	RECT rect;
	GetClientRect(m_hAppWnd, &rect);
	UINT width = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	// initialize matrix 
	m_World = XMMatrixIdentity();

	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 150.0f, -350.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 150.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	m_View = XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV4, width / (float)height, 0.01f, 10000.0f);

	// Update our time
	static float t = 0.0f;
	if (m_DriverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();
		if (dwTimeStart == 0)
		{
			dwTimeStart = dwTimeCur;
		}
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
	}

	// Rotate cube around the origin
	m_World = XMMatrixRotationY(t);

	//
	// Clear the back buffer
	//
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red, green, blue, alpha
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
	//
	// Clear the depth buffer to 1.0 (max depth)
	//
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	float blendFactors[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	m_pImmediateContext->RSSetState(m_RasterizerState);
	m_pImmediateContext->OMSetBlendState(m_BlendState, blendFactors, 0xffffffff);
	m_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState, 0);

	//
	m_pUserAnnotation->BeginEvent(L"Model_Draw");

	size_t nodeCount = m_FBXRenderer->GetNodeCount();
	m_pUserAnnotation->BeginEvent(L"Model_Draw_Node_Count");

	ID3D11VertexShader* vertex = m_VertexShader;
	m_pImmediateContext->VSSetShader(vertex, NULL, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_PcBuffer);
	m_pImmediateContext->PSSetShader(m_PixelShader, NULL, 0);



	//OutputDebugString("\n NodeCountÅF%d \n", nodeCount);
	// allnode
	for (int i = 0; i < nodeCount; ++i)
	{
		XMMATRIX mLocal;
		m_FBXRenderer->GetNodeMatrix(i, &mLocal.r[0].m128_f32[0]);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		m_pImmediateContext->Map(m_PcBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

		ConstantBufferMatrix* cbFBX = (ConstantBufferMatrix*)MappedResource.pData;

		// ç∂éËån
		cbFBX->world = (m_World);
		cbFBX->view  = (m_View);
		cbFBX->proj  = (m_Projection);
		cbFBX->wvp   = XMMatrixTranspose(mLocal * m_World * m_View * m_Projection);
		m_pImmediateContext->Unmap(m_PcBuffer, 0);

		SetMatrix();

		MaterialData material = m_FBXRenderer->GetNodeMaterial(i);

		if (material.pMaterialCb)
		{
			m_pImmediateContext->UpdateSubresource(material.pMaterialCb, 0, NULL, &material.materialConstantData, 0, 0);
		}

		m_pImmediateContext->VSSetShaderResources(0, 1, &m_TransformSRV);
		m_pImmediateContext->PSSetShaderResources(0, 1, &material.pSRV);
		m_pImmediateContext->PSSetConstantBuffers(0, 1, &material.pMaterialCb);
		m_pImmediateContext->PSSetSamplers(0, 1, &material.pSampler);

#if false
		if (false)
		{
			m_FBXRenderer->RenderNodeInstancing(m_pImmediateContext, i, m_InstanceMAX);
		}
		else
		{
		}
#endif
		m_FBXRenderer->RenderNode(m_pImmediateContext, i);


	}

	// Text
	WCHAR wstr[512];
	//g_pSpriteBatch->Begin();
	//g_pFont->DrawString(g_pSpriteBatch, L"FBX Loader : F2 Change Render Mode", XMFLOAT2(0, 0), DirectX::Colors::Yellow, 0, XMFLOAT2(0, 0), 0.5f);

	//if (g_bInstancing)
	//{
	//	swprintf_s(wstr, L"Render Mode: Instancing");
	//}
	//else
	//{
	//	swprintf_s(wstr, L"Render Mode: Single Draw");
	//}
	//g_pFont->DrawString(g_pSpriteBatch, wstr, XMFLOAT2(0, 16), DirectX::Colors::Yellow, 0, XMFLOAT2(0, 0), 0.5f);
	//g_pSpriteBatch->End();

	m_pImmediateContext->VSSetShader(NULL, NULL, 0);
	m_pImmediateContext->PSSetShader(NULL, NULL, 0);

	DXApp::Render();
	//m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, Colors::Black);
	HR(m_pSwapChain->Present(0, 0));
}

void Application::SetMatrix()
{
	HRESULT hr = S_OK;
	const uint32_t count = m_InstanceMAX;
	const float offset = -(m_InstanceMAX*60.0f / 2.0f);
	XMMATRIX mat;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = m_pImmediateContext->Map(m_TransformStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

	SRVPerInstanceData*	pSrvInstanceData = (SRVPerInstanceData*)MappedResource.pData;

	for (uint32_t i = 0; i<count; i++)
	{
		mat = XMMatrixTranslation(0, 0, i*60.0f + offset);
		pSrvInstanceData[i].World = (mat);
	}

	m_pImmediateContext->Unmap(m_TransformStructuredBuffer, 0);
}
