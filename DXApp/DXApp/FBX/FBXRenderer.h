#pragma once
#include "FBXLoader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace FBX;

namespace FBX
{
	struct VertexData
	{
		XMFLOAT3 vPos;
		XMFLOAT3 vNor;
		XMFLOAT2 vTexcoord;
	};

	struct MaterialConstantData
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT4 emmisive;
	};

	struct MaterialData
	{
		DirectX::XMFLOAT4	ambient;
		DirectX::XMFLOAT4	diffuse;
		DirectX::XMFLOAT4	specular;
		DirectX::XMFLOAT4	emmisive;
		float specularPower;
		float TransparencyFactor;

		MaterialConstantData materialConstantData;

		ID3D11ShaderResourceView*	pSRV;
		ID3D11SamplerState*         pSampler;
		ID3D11Buffer*				pMaterialCb;

		MaterialData()
		{
			pSRV = nullptr;
			pSampler = nullptr;
			pMaterialCb = nullptr;
		}

		void Release()
		{
			if (pMaterialCb)
			{
				pMaterialCb->Release();
				pMaterialCb = nullptr;
			}

			if (pSRV)
			{
				pSRV->Release();
				pSRV = nullptr;
			}

			if (pSampler)
			{
				pSampler->Release();
				pSampler = nullptr;
			}
		}
	};


	struct MeshNode
	{
		ID3D11Buffer* m_pVB;
		ID3D11Buffer* m_pIB;
		ID3D11InputLayout* m_pInputLayout;

		DWORD vertexCount;
		DWORD indexCount;

		MaterialData materialData;

		float mat4x4[16];

		// INDEX BUFFERのBIT
		enum INDEX_BIT
		{
			INDEX_NOINDEX = 0,
			INDEX_16BIT, // 16bitインデックス
			INDEX_32BIT, // 32bitインデックス
		};
		INDEX_BIT	m_indexBit;

		MeshNode()
		{
			m_pVB = nullptr;
			m_pIB = nullptr;
			m_pInputLayout = nullptr;
			m_indexBit = INDEX_NOINDEX;
			vertexCount = 0;
			indexCount = 0;
		}

		void Release()
		{
			materialData.Release();

			if (m_pInputLayout)
			{
				m_pInputLayout->Release();
				m_pInputLayout = nullptr;
			}
			if (m_pIB)
			{
				m_pIB->Release();
				m_pIB = nullptr;
			}
			if (m_pVB)
			{
				m_pVB->Release();
				m_pVB = nullptr;
			}
		}

		void SetIndexBit(const size_t indexCount)
		{
			m_indexBit = INDEX_NOINDEX;
			if (indexCount != 0)
			{
				m_indexBit = INDEX_32BIT;
			}
		};
	};

	class FBXRenderer
	{

	private:
		FBXLoader * m_Loader;
		std::vector<MeshNode> m_MeshNodeArray;

		HRESULT CreateNodes(ID3D11Device* pd3dDevice);
		HRESULT VertexConstruction(ID3D11Device* pd3dDevice, FBXMeshNode& fbxNode, MeshNode& meshNode);
		HRESULT MaterialConstruction(ID3D11Device* pd3dDevice, FBXMeshNode& fbxNode, MeshNode& meshNode);

		HRESULT CreateVertexBuffer(ID3D11Device* pd3dDevice, ID3D11Buffer** pBuffer, void* pVertices, uint32_t stride, uint32_t vertexCount);
		HRESULT CreateIndexBuffer(ID3D11Device* pd3dDevice, ID3D11Buffer** pBuffer, void* pIndices, uint32_t indexCount);

	public:
		FBXRenderer();
		~FBXRenderer();

		void Release();

		HRESULT LoadFBX(const char* fileName, ID3D11Device* pd3dDevice);
		HRESULT CreateInputLayout(ID3D11Device*	pd3dDevice, const void* pShaderBytecodeWithInputSignature, size_t BytecodeLength, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int layoutSize);

		HRESULT RenderAll(ID3D11DeviceContext* pImmediateContext);
		HRESULT RenderNode(ID3D11DeviceContext* pImmediateContext, const size_t nodeId);
		HRESULT RenderNodeInstancing(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, const uint32_t InstanceCount);
		HRESULT RenderNodeInstancingIndirect(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11Buffer* pBufferForArgs, const uint32_t AlignedByteOffsetForArgs);

		size_t GetNodeCount() 
		{ 
			return m_MeshNodeArray.size(); 
		}

		MeshNode& GetNode(const int id) 
		{
			return m_MeshNodeArray[id];
		};

		void GetNodeMatrix(const int id, float* mat4x4) 
		{ 
			memcpy(mat4x4, m_MeshNodeArray[id].mat4x4, sizeof(float) * 16);
		};

		MaterialData& GetNodeMaterial(const size_t id) 
		{
			return m_MeshNodeArray[id].materialData;
		};
	};

}


