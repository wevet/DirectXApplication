#pragma once
#include <memory>
#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include <d3d11_1.h>
#include <stdio.h>
#include <string>
#include "DXApp.h"
#include "GameObject\Actor.h"

#ifdef _DEBUG
#pragma comment(lib, "DirectXTK_d.lib")
#else
#pragma comment(lib, "DirectXTK.lib")
#endif

#include "SpriteBatch.h"
#include "DDSTextureLoader.h"
#include "SimpleMath.h"
#include "FBX\FBXRenderer.h"

using namespace DirectX;
using namespace Microsoft;
using namespace GameObject;
using namespace FBX;

namespace Prototype
{

	class Application : public DXApp
	{
	public:
		Application(HINSTANCE hInstace);
		~Application();

		bool Initialize() override;
		void Update(float dt) override;
		void Render() override;

	protected:
		virtual void InitDirect3DInternal() override;
		virtual HRESULT InitApp() override;
		virtual HRESULT SetupTransformSRV() override;

	private:
		std::unique_ptr<SpriteBatch> spriteBatch;
		WRL::ComPtr<ID3D11ShaderResourceView> m_pTexture;
		SimpleMath::Vector2 m_pos;
		FBXRenderer* m_FBXRenderer;

		void SetMatrix();
	};

}

