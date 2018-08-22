#include <tchar.h>
#include "FBXLoader.h"
#include "../DXUtil.h"

using namespace PrototypeTools;


FBXLoader::FBXLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, D3D11_VIEWPORT viewport)
{
	m_pDeviceContext = pDeviceContext;
	m_pDevice   = pDevice;
	m_pRenderTargetView = pRenderTargetView;
	m_pViewport = viewport;
	fbxManager  = FbxManager::Create();
	fbxScene    = FbxScene::Create(fbxManager, "fbxscene");
	startOffsetX = 0;
}

FBXLoader::~FBXLoader()
{
	pRasterizerState->Release();
	pVertexShader->Release();
	pVertexLayout->Release();
	pPixelShader->Release();
	pConstantBuffer->Release();
	verBuffer->Release();
	indBuffer->Release();
	fbxScene->Destroy();
	fbxManager->Destroy();
	delete[] vertices;
	Memory::SafeRelease(m_pDevice);
	Memory::SafeRelease(m_pDeviceContext);
	Memory::SafeRelease(m_pRenderTargetView);
}

bool FBXLoader::Initialize()
{
	if (fbxManager == nullptr || fbxScene == nullptr)
	{
		OutputDebugString("\n nullptr fbxManager | fbxScene \n");
		return false;
	}

	// シェーダの設定
	//ID3DBlob *pCompileVS = NULL;
	//ID3DBlob *pCompilePS = NULL;
	//D3DCompileFromFile(L"Shaders/Sample.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	//m_pDevice->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader);
	//D3DCompileFromFile(L"Shaders/Sample.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	//m_pDevice->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader);

	// 頂点レイアウト
	//D3D11_INPUT_ELEMENT_DESC layout[] = {
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//};
	//m_pDevice->CreateInputLayout(layout, 1, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout);
	//pCompileVS->Release();
	//pCompilePS->Release();

	// 定数バッファの設定
	//D3D11_BUFFER_DESC cb;
	//cb.ByteWidth = sizeof(CONSTANTBUFFER);
	//cb.Usage = D3D11_USAGE_DYNAMIC;
	//cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//cb.MiscFlags = 0;
	//cb.StructureByteStride = 0;
	//m_pDevice->CreateBuffer(&cb, NULL, &pConstantBuffer);

	// fbxの読み込み
	FbxString FileName("Assets/FBX/Miku_default/Miku_default.fbx");
	FbxImporter *fbxImporter = FbxImporter::Create(fbxManager, "imp");

	bool importResult = fbxImporter->Initialize(FileName, -1, fbxManager->GetIOSettings());
	if (!importResult)
	{
		_stprintf_s(DebugStr, 512, _T("\n ■□■ fbx file load error FileName: [ %s ] ■□■\n"), FileName);
		OutputDebugString(DebugStr);
		return false;
	}

	bool importSceneResult = fbxImporter->Import(fbxScene);
	if (!importSceneResult)
	{
		_stprintf_s(DebugStr, 512, _T("\n ■□■ ERROR: import error FileName: [ %s ] ■□■\n"), FileName);
		OutputDebugString(DebugStr);
		return false;
	}
	if (fbxImporter)
	{
		fbxImporter->Destroy();
		fbxImporter = nullptr;
	}

	// get node 
	LoadNode(fbxScene->GetRootNode());

	if (meshArray.size() <= 0)
	{
		OutputDebugString("\n not found mesh \n");
		return false;
	}

	_stprintf_s(DebugStr, 512, _T("\n ■□■ Log: get mesh array success ArrayCount %d ■□■ \n"), meshArray.size());
	OutputDebugString(DebugStr);

	for (auto mesh : meshArray)
	{
		LoadMesh(mesh);
	}
	return true;
}

void FBXLoader::Render(float dt)
{
	// パラメータの計算
	//XMVECTOR eye_pos    = XMVectorSet(0.0f, 70.0f, 500.0f, 1.0f);
	//XMVECTOR eye_lookat = XMVectorSet(0.0f, 70.0f, 0.0f, 1.0f);
	//XMVECTOR eye_up     = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	//XMMATRIX World = XMMatrixRotationY(startOffsetX += 0.001);
	//XMMATRIX View  = XMMatrixLookAtLH(eye_pos, eye_lookat, eye_up);
	//XMMATRIX Proj  = XMMatrixPerspectiveFovLH(XM_PIDIV4, m_pViewport.Width / m_pViewport.Height, 0.1f, 800.0f);

	// パラメータの受け渡し
	//D3D11_MAPPED_SUBRESOURCE pdata;
	//CONSTANTBUFFER cb;
	//cb.mWVP = XMMatrixTranspose(World * View * Proj);
	//m_pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	//memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
	//m_pDeviceContext->Unmap(pConstantBuffer, 0);

	for (auto mesh : meshArray)
	{
		m_pDeviceContext->DrawIndexed(mesh->GetPolygonVertexCount(), 0, 0);
	}
}

void FBXLoader::LoadNode(FbxNode * fbxNode)
{
	int numAttributes = fbxNode->GetNodeAttributeCount();
	for (int i = 0; i < numAttributes; i++)
	{
		FbxNodeAttribute *nodeAttributeFbx = fbxNode->GetNodeAttributeByIndex(i);
		FbxNodeAttribute::EType attributeType = nodeAttributeFbx->GetAttributeType();

		switch (attributeType)
		{
			case FbxNodeAttribute::eMesh:
			{
				// Load keyframe transformations
				// LoadNodeKeyframeAnimation(fbxNode);

				// Load mesh vertices, texcoords, normals, etc
				// LoadMesh((FbxMesh*)nodeAttributeFbx);

				// Load mesh skeleton
				// LoadMesh_Skeleton((FbxMesh*)nodeAttributeFbx);

				meshArray.push_back((FbxMesh*)nodeAttributeFbx);
				break;
			}
		}
	}

	// Load the child nodes
	int numChildren = fbxNode->GetChildCount();
	for (int i = 0; i < numChildren; i++)
	{
		LoadNode(fbxNode->GetChild(i));
	}
}

void FBXLoader::LoadMesh(FbxMesh * mesh)
{
	if (mesh == nullptr)
	{
		return;
	}

	vertices = new VERTEX[mesh->GetControlPointsCount()];
	for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
		vertices[i].Pos.x = (FLOAT)mesh->GetControlPointAt(i)[0];
		vertices[i].Pos.y = (FLOAT)mesh->GetControlPointAt(i)[1];
		vertices[i].Pos.z = (FLOAT)mesh->GetControlPointAt(i)[2];
	}

	HRESULT hr;
	// 頂点データ用バッファの設定
	D3D11_BUFFER_DESC bd_vertex;
	bd_vertex.ByteWidth = sizeof(VERTEX) * mesh->GetControlPointsCount();
	bd_vertex.Usage = D3D11_USAGE_DEFAULT;
	bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd_vertex.CPUAccessFlags = 0;
	bd_vertex.MiscFlags = 0;
	bd_vertex.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_vertex;
	data_vertex.pSysMem = vertices;
	hr = m_pDevice->CreateBuffer(&bd_vertex, &data_vertex, &verBuffer);
	if (FAILED(hr))
	{
		OutputDebugString("\n load error vertex buffer \n");
		return;
	}

	// インデックスデータの取り出しとバッファの設定
	D3D11_BUFFER_DESC bd_index;
	bd_index.ByteWidth = sizeof(int) * mesh->GetPolygonVertexCount();
	bd_index.Usage = D3D11_USAGE_DEFAULT;
	bd_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd_index.CPUAccessFlags = 0;
	bd_index.MiscFlags = 0;
	bd_index.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA data_index;
	data_index.pSysMem = mesh->GetPolygonVertices();
	hr = m_pDevice->CreateBuffer(&bd_index, &data_index, &indBuffer);
	if (FAILED(hr))
	{
		OutputDebugString("\n load error index buffer \n");
		return;
	}

	// ラスタライザの設定
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;
	hr = m_pDevice->CreateRasterizerState(&rdc, &pRasterizerState);
	if (FAILED(hr))
	{
		OutputDebugString("\n load error rasterizer \n");
		return;
	}

	// オブジェクトの反映
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	m_pDeviceContext->IASetVertexBuffers(0, 1, &verBuffer, &stride, &offset);
	m_pDeviceContext->IASetIndexBuffer(indBuffer, DXGI_FORMAT_R32_UINT, 0);
	m_pDeviceContext->IASetInputLayout(pVertexLayout);
	m_pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	m_pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	m_pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	m_pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
	m_pDeviceContext->RSSetState(pRasterizerState);
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, NULL);
	m_pDeviceContext->RSSetViewports(1, &m_pViewport);
}
