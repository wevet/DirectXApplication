#include <tchar.h>
#include "FBXLoader.h"
#include "../DXUtil.h"

using namespace PrototypeTools;


FBXLoader::FBXLoader()
{
	fbxManager = FbxManager::Create();
	fbxScene   = FbxScene::Create(fbxManager, "fbxscene");
}

FBXLoader::~FBXLoader()
{
	fbxManager->Destroy();
}

bool FBXLoader::Create()
{
	return false;
}

bool FBXLoader::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, D3D11_VIEWPORT viewport)
{
	if (pDevice == nullptr || pDeviceContext == nullptr)
	{
		OutputDebugString("\n Not Found Device | DeviceContext \n");
		return false;
	}

	if (fbxManager == nullptr || fbxScene == nullptr)
	{
		OutputDebugString("\n nullptr fbxManager | fbxScene \n");
		return false;
	}

	/*
	// シェーダの設定
	ID3DBlob *pCompileVS = NULL;
	ID3DBlob *pCompilePS = NULL;
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	pDevice->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader);
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	pDevice->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader);

	// 頂点レイアウト
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	pDevice->CreateInputLayout(layout, 1, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout);
	pCompileVS->Release();
	pCompilePS->Release();
	*/

	// 定数バッファの設定
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANTBUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	pDevice->CreateBuffer(&cb, NULL, &pConstantBuffer);

	// fbxの読み込み
	FbxString FileName("Assets/FBX/Miku_default/Miku_default.fbx");
	FbxImporter *fbxImporter = FbxImporter::Create(fbxManager, "Scene");
	bool importResult = fbxImporter->Initialize(FileName.Buffer(), -1, fbxManager->GetIOSettings());
	if (!importResult)
	{
		_stprintf_s(DebugStr, 512, _T("■□■ FBXファイルロードエラー. FileName: [ %s ] ■□■\n"), FileName);
		OutputDebugString(DebugStr);
		return false;
	}

	// インポート対象とする要素を指定してると思うが、IMP_FBX_TEXTUREをfalseにしてもテクスチャーを普通にロードできる。
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
	//fbxManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	bool importSceneResult = fbxImporter->Import(fbxScene);
	if (!importSceneResult)
	{
		_stprintf_s(DebugStr, 512, _T("■□■ ERROR: インポートエラー. FileName: [ %s ] ■□■\n"), FileName);
		OutputDebugString(DebugStr);
		return false;
	}
	if (fbxImporter)
	{
		fbxImporter->Destroy();
		fbxImporter = nullptr;
	}

	// 頂点データの取り出し
	for (int i = 0; i < fbxScene->GetRootNode()->GetChildCount(); i++) 
	{
		FbxNode* node = fbxScene->GetRootNode()->GetChild(i);
		if (node == nullptr)
		{
			continue;
		}
		FbxNodeAttribute* attribute = node->GetNodeAttribute();
		if (attribute == nullptr)
		{
			continue;
		}

		_stprintf_s(DebugStr, 512, _T("■□■ Log: NodeName [ %s ] ■□■\n"), node->GetName());
		OutputDebugString(DebugStr);

		_stprintf_s(DebugStr, 512, _T("■□■ Log: AttributeName [ %s ] ■□■\n"), attribute->GetName());
		OutputDebugString(DebugStr);

		_stprintf_s(DebugStr, 512, _T("■□■ Log: AttributeType [ %d ] ■□■\n"), attribute->GetAttributeType());
		OutputDebugString(DebugStr);

		FbxNodeAttribute::EType type = attribute->GetAttributeType();

		//if (type == FbxNodeAttribute::eMesh || type == FbxNodeAttribute::eNurbs || type == FbxNodeAttribute::eNurbsSurface || type == FbxNodeAttribute::ePatch || type == FbxNodeAttribute::eSkeleton)
		if (type == FbxNodeAttribute::eSkeleton)
		{
			mesh = node->GetMesh();
			break;
		}
	}

	if (mesh == nullptr)
	{
		OutputDebugString("\n not found mesh \n");
		return false;
	}

	vertices = new VERTEX[mesh->GetControlPointsCount()];
	for (int i = 0; i < mesh->GetControlPointsCount(); i++) 
	{
		vertices[i].Pos.x = (FLOAT)mesh->GetControlPointAt(i)[0];
		vertices[i].Pos.y = (FLOAT)mesh->GetControlPointAt(i)[1];
		vertices[i].Pos.z = (FLOAT)mesh->GetControlPointAt(i)[2];
	}

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
	pDevice->CreateBuffer(&bd_vertex, &data_vertex, &verBuffer);

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
	pDevice->CreateBuffer(&bd_index, &data_index, &indBuffer);

	// ラスタライザの設定
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;
	pDevice->CreateRasterizerState(&rdc, &pRasterizerState);

	// オブジェクトの反映
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &verBuffer, &stride, &offset);
	pDeviceContext->IASetIndexBuffer(indBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetInputLayout(pVertexLayout);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
	pDeviceContext->RSSetState(pRasterizerState);
	pDeviceContext->OMSetRenderTargets(1, &pBackBuffer_RTV, NULL);
	pDeviceContext->RSSetViewports(1, &viewport);

	return true;
}
