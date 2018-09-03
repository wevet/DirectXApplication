#include "FBXRenderer.h"
#include <locale.h>
#include <tchar.h>
#include <string>
#include "../DXUtil.h"
#include "DDSTextureLoader.h"

using namespace FBX;

FBXRenderer::FBXRenderer()
{
	m_Loader = nullptr;
}

FBXRenderer::~FBXRenderer()
{
	Release();
}

void FBXRenderer::Release()
{
	for (size_t i = 0; i < m_MeshNodeArray.size(); i++)
	{
		m_MeshNodeArray[i].Release();
	}
	m_MeshNodeArray.clear();

	if (m_Loader)
	{
		Memory::SafeRelease(m_Loader);
	}
}

HRESULT FBXRenderer::LoadFBX(const char * fileName, ID3D11Device * pd3dDevice)
{
	if (!fileName || !pd3dDevice)
	{
		return E_FAIL;
	}
	OutputDebugString(_T(fileName));

	HRESULT hr = S_OK;

	m_Loader = new FBXLoader;
	hr = m_Loader->LoadFBX(fileName, FBXLoader::EAxisSystem::Directx);
	if (FAILED(hr))
	{
		OutputDebugString("\n Error Loader \n");
		return hr;
	}

	hr = CreateNodes(pd3dDevice);
	if (FAILED(hr))
	{
		OutputDebugString("\n Error Create Node \n");
		return hr;
	}

	return hr;
}

HRESULT FBXRenderer::CreateNodes(ID3D11Device * pd3dDevice)
{
	if (!pd3dDevice)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	size_t nodeCoount = m_Loader->GetNodesCount();
	if (nodeCoount == 0)
	{
		return E_FAIL;
	}

	_stprintf_s(DebugStr, 512, _T("■□■ nodeCoount: [ %d ] ■□■\n"), nodeCoount);
	OutputDebugString(DebugStr);

	for (int i = 0; i < nodeCoount; ++i)
	{
		MeshNode meshNode;
		FBXMeshNode fbxNode = m_Loader->GetNode(static_cast<unsigned int>(i));
		VertexConstruction(pd3dDevice, fbxNode, meshNode);

		// index buffer
		meshNode.indexCount = static_cast<DWORD>(fbxNode.indexArray.size());
		meshNode.SetIndexBit(meshNode.indexCount);
		if (fbxNode.indexArray.size() > 0)
		{
			hr = CreateIndexBuffer(pd3dDevice, &meshNode.m_pIB, &fbxNode.indexArray[0], static_cast<uint32_t>(fbxNode.indexArray.size()));
		}

		memcpy(meshNode.mat4x4, fbxNode.matrix4x4, sizeof(float) * 16);
		MaterialConstruction(pd3dDevice, fbxNode, meshNode);
		m_MeshNodeArray.push_back(meshNode);
	}

	return hr;
}

HRESULT FBXRenderer::CreateVertexBuffer(ID3D11Device* pd3dDevice, ID3D11Buffer** pBuffer, void* pVertices, uint32_t stride, uint32_t vertexCount)
{
	if (!pd3dDevice || stride == 0 || vertexCount == 0)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth = stride * vertexCount;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pVertices;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

HRESULT FBXRenderer::CreateIndexBuffer(ID3D11Device* pd3dDevice, ID3D11Buffer** pBuffer, void* pIndices, uint32_t indexCount)
{
	if (!pd3dDevice || indexCount == 0)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	size_t stride = sizeof(unsigned int);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = static_cast<uint32_t>(stride*indexCount);
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	InitData.pSysMem = pIndices;

	hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

HRESULT FBXRenderer::VertexConstruction(ID3D11Device * pd3dDevice, FBXMeshNode & fbxNode, MeshNode & meshNode)
{

	meshNode.vertexCount = static_cast<DWORD>(fbxNode.positionArray.size());
	if (!pd3dDevice || meshNode.vertexCount == 0)
	{
		return E_FAIL;
	}
	HRESULT hr = S_OK;

	_stprintf_s(DebugStr, 512, _T("■□■ VertexConstruction vertex count: [ %d ] ■□■\n"), meshNode.vertexCount);
	OutputDebugString(DebugStr);
	
	VertexData* pV = new VertexData[meshNode.vertexCount];

	for (size_t i = 0; i < meshNode.vertexCount; i++)
	{
		FbxVector4 v = fbxNode.positionArray[i];

		_stprintf_s(DebugStr, 512, _T("■□■ FbxVector4: [ %lf, %lf, %lf ] ■□■\n"), (float)v.mData[0], (float)v.mData[1], (float)v.mData[2]);
		OutputDebugString(DebugStr);

		pV[i].vPos = XMFLOAT3(
			(float)v.mData[0],
			(float)v.mData[1],
			(float)v.mData[2]);

		v = fbxNode.normalArray[i];

		pV[i].vNor = XMFLOAT3(
			(float)v.mData[0],
			(float)v.mData[1],
			(float)v.mData[2]);

		if ((float)fbxNode.texcoordArray.size() > 0)
		{
			pV[i].vTexcoord = XMFLOAT2(
				(float)abs(1.0f - fbxNode.texcoordArray[i].mData[0]),
				(float)abs(1.0f - fbxNode.texcoordArray[i].mData[1]));
		}
		else
		{
			pV[i].vTexcoord = XMFLOAT2(0, 0);
		}
	}

	CreateVertexBuffer(pd3dDevice, &meshNode.m_pVB, pV, sizeof(VertexData), meshNode.vertexCount);

	if (pV)
	{
		Memory::SafeDeleteArray(pV);
	}

	return hr;
}

HRESULT FBXRenderer::MaterialConstruction(ID3D11Device * pd3dDevice, FBXMeshNode & fbxNode, MeshNode & meshNode)
{
	if (!pd3dDevice || fbxNode.materialArray.size() == 0)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;

	_stprintf_s(DebugStr, 512, _T("■□■ materialArray size: [ %zd ] ■□■\n"), fbxNode.materialArray.size());
	OutputDebugString(DebugStr);

	for (int i = 0; i < fbxNode.materialArray.size(); ++i)
	{
		FBXMaterialNode fbxMaterial = fbxNode.materialArray[i];
		meshNode.materialData.specularPower = fbxMaterial.shininess;
		meshNode.materialData.TransparencyFactor = fbxMaterial.transparencyFactor;
		meshNode.materialData.ambient = XMFLOAT4(fbxMaterial.ambient.r, fbxMaterial.ambient.g, fbxMaterial.ambient.b, fbxMaterial.ambient.a);
		meshNode.materialData.diffuse = XMFLOAT4(fbxMaterial.diffuse.r, fbxMaterial.diffuse.g, fbxMaterial.diffuse.b, fbxMaterial.diffuse.a);
		meshNode.materialData.specular = XMFLOAT4(fbxMaterial.specular.r, fbxMaterial.specular.g, fbxMaterial.specular.b, fbxMaterial.specular.a);
		meshNode.materialData.emmisive = XMFLOAT4(fbxMaterial.emmisive.r, fbxMaterial.emmisive.g, fbxMaterial.emmisive.b, fbxMaterial.emmisive.a);

		// Diffuseだけからテクスチャを読み込む
		if (fbxMaterial.diffuse.textureSetArray.size() > 0)
		{
			TextureSet::const_iterator it = fbxMaterial.diffuse.textureSetArray.begin();
			if (it->second.size())
			{
				std::string path = it->second[0];
				WCHAR wstr[512];
				size_t wLen = 0;
				mbstowcs_s(&wLen, wstr, path.size() + 1, path.c_str(), _TRUNCATE);

				_stprintf_s(DebugStr, 512, _T("■□■ szFileName: [ %ls ] ■□■\n"), wstr);
				OutputDebugString(DebugStr);
				CreateDDSTextureFromFile(pd3dDevice, wstr, NULL, &meshNode.materialData.pSRV, 0);// DXTexから
			}
		}
	}

	// samplerstate
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pd3dDevice->CreateSamplerState(&sampDesc, &meshNode.materialData.pSampler);

	// material Constant Buffer
	D3D11_BUFFER_DESC bufDesc;
	ZeroMemory(&bufDesc, sizeof(bufDesc));
	bufDesc.ByteWidth = sizeof(MaterialConstantData);
	bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = 0;

	hr = pd3dDevice->CreateBuffer(&bufDesc, NULL, &meshNode.materialData.pMaterialCb);
	meshNode.materialData.materialConstantData.ambient  = meshNode.materialData.ambient;
	meshNode.materialData.materialConstantData.diffuse  = meshNode.materialData.ambient;
	meshNode.materialData.materialConstantData.specular = meshNode.materialData.specular;
	meshNode.materialData.materialConstantData.emmisive = meshNode.materialData.emmisive;
	return hr;
}

HRESULT FBXRenderer::CreateInputLayout(ID3D11Device* pd3dDevice, const void* pShaderBytecodeWithInputSignature, size_t BytecodeLength, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int layoutSize)
{
	// InputeLayoutは頂点シェーダのコンパイル結果が必要
	if (!pd3dDevice || !pShaderBytecodeWithInputSignature || !pLayout)
	{
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	size_t nodeCount = m_MeshNodeArray.size();

	for (size_t i = 0; i<nodeCount; i++)
	{
		pd3dDevice->CreateInputLayout(pLayout, layoutSize, pShaderBytecodeWithInputSignature, BytecodeLength, &m_MeshNodeArray[i].m_pInputLayout);
	}
	return hr;
}

HRESULT FBXRenderer::RenderAll(ID3D11DeviceContext* pImmediateContext)
{
	size_t nodeCount = m_MeshNodeArray.size();
	if (nodeCount == 0)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;

	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (size_t i = 0; i<nodeCount; i++)
	{
		MeshNode* node = &m_MeshNodeArray[i];

		if (node->vertexCount == 0)
		{
			continue;
		}

		UINT stride = sizeof(VertexData);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);

		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node->m_indexBit == MeshNode::INDEX_32BIT)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		pImmediateContext->IASetInputLayout(node->m_pInputLayout);
		pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);
		pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
	}
	return hr;
}

HRESULT FBXRenderer::RenderNode(ID3D11DeviceContext* pImmediateContext, const size_t nodeId)
{
	size_t nodeCount = m_MeshNodeArray.size();
	if (nodeCount == 0 || nodeCount <= nodeId)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;

	MeshNode* node = &m_MeshNodeArray[nodeId];

	if (node->vertexCount == 0)
	{
		return S_OK;
	}

	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
	pImmediateContext->IASetInputLayout(node->m_pInputLayout);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// インデックスバッファが存在する場合
	if (node->m_indexBit != MeshNode::INDEX_NOINDEX)
	{
		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node->m_indexBit == MeshNode::INDEX_32BIT)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);
		pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
	}

	return hr;
}

HRESULT FBXRenderer::RenderNodeInstancing(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, const uint32_t InstanceCount)
{
	size_t nodeCount = m_MeshNodeArray.size();
	if (nodeCount == 0 || nodeCount <= nodeId || InstanceCount == 0)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;

	MeshNode* node = &m_MeshNodeArray[nodeId];

	if (node->vertexCount == 0)
	{
		return S_OK;
	}

	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
	pImmediateContext->IASetInputLayout(node->m_pInputLayout);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// インデックスバッファが存在する場合
	if (node->m_indexBit != MeshNode::INDEX_NOINDEX)
	{
		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node->m_indexBit == MeshNode::INDEX_32BIT)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);
		pImmediateContext->DrawIndexedInstanced(node->indexCount, InstanceCount, 0, 0, 0);
	}

	return hr;
}

HRESULT FBXRenderer::RenderNodeInstancingIndirect(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11Buffer* pBufferForArgs, const uint32_t AlignedByteOffsetForArgs)
{
	size_t nodeCount = m_MeshNodeArray.size();
	if (nodeCount == 0 || nodeCount <= nodeId)
	{
		return S_OK;
	}

	HRESULT hr = S_OK;
	MeshNode* node = &m_MeshNodeArray[nodeId];

	if (node->vertexCount == 0)
	{
		return S_OK;
	}

	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
	pImmediateContext->IASetInputLayout(node->m_pInputLayout);
	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// インデックスバッファが存在する場合
	if (node->m_indexBit != MeshNode::INDEX_NOINDEX)
	{
		DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
		if (node->m_indexBit == MeshNode::INDEX_32BIT)
		{
			indexbit = DXGI_FORMAT_R32_UINT;
		}
		pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);
		pImmediateContext->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
	}

	return hr;
}

