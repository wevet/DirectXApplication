#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>
#include <vector>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace PrototypeTools
{
	// 一つの頂点情報を格納する構造体
	struct VERTEX
	{
		XMFLOAT3 Pos;
	};

	// GPU(シェーダ側)へ送る数値をまとめた構造体
	struct CONSTANTBUFFER
	{
		XMMATRIX mWVP;
	};

	class FBXLoader
	{
	public:
		FBXLoader(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RenderTargetView* pRenderTargetView, D3D11_VIEWPORT viewport);
		~FBXLoader();

		bool Initialize();
		void Render(float dt);

	private:
		ID3D11RasterizerState*  pRasterizerState;
		ID3D11VertexShader*     pVertexShader;
		ID3D11InputLayout*      pVertexLayout;
		ID3D11PixelShader*      pPixelShader;
		ID3D11Buffer*           pConstantBuffer;

		ID3D11RenderTargetView* m_pRenderTargetView;
		ID3D11Device*           m_pDevice;
		ID3D11DeviceContext*    m_pDeviceContext;
		D3D11_VIEWPORT          m_pViewport;

		FbxManager*   fbxManager = NULL;
		FbxScene*     fbxScene   = NULL;
		ID3D11Buffer* verBuffer  = NULL;
		ID3D11Buffer* indBuffer  = NULL;
		VERTEX*       vertices;

		std::vector<FbxMesh*> meshArray;

		// デバック用文字列
		TCHAR DebugStr[512];

		void LoadNode(FbxNode* fbxNode);
		void LoadMesh(FbxMesh* mesh);
		float startOffsetX;
	};

}


