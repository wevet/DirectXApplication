#pragma once
#include <Windows.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>

using namespace DirectX;

//#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dcompiler.lib")

namespace PrototypeTools
{
	// ��̒��_�����i�[����\����
	struct VERTEX
	{
		XMFLOAT3 Pos;
	};

	// GPU(�V�F�[�_��)�֑��鐔�l���܂Ƃ߂��\����
	struct CONSTANTBUFFER
	{
		XMMATRIX mWVP;
	};

	class FBXLoader
	{
	public:
		FBXLoader();
		~FBXLoader();

		bool Create();
		bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, D3D11_VIEWPORT viewport);

	private:
		ID3D11RenderTargetView* pBackBuffer_RTV;
		ID3D11RasterizerState*  pRasterizerState;
		ID3D11VertexShader*     pVertexShader;
		ID3D11InputLayout*      pVertexLayout;
		ID3D11PixelShader*      pPixelShader;
		ID3D11Buffer*           pConstantBuffer;

		FbxManager*   fbxManager = NULL;
		FbxScene*     fbxScene   = NULL;
		FbxMesh*      mesh       = NULL;
		ID3D11Buffer* verBuffer  = NULL;
		ID3D11Buffer* indBuffer  = NULL;
		VERTEX*       vertices;

		// �f�o�b�N�p������
		TCHAR DebugStr[512];
	};

}


