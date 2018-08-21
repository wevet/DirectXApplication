#pragma once
#include <memory>
#include <windows.h>
#include <tchar.h>
#include <wrl.h>
#include <d3d11_1.h>
#include "DXApp.h"

#ifdef _DEBUG
#pragma comment(lib, "DirectXTK_d.lib")
#else
#pragma comment(lib, "DirectXTK.lib")
#endif

#include "SpriteBatch.h"
#include "DDSTextureLoader.h"
#include "SimpleMath.h"


using namespace DirectX;
using namespace Microsoft;

namespace Prototype
{

	class Application : public DXApp
	{
	public:
		Application(HINSTANCE hInstace);
		~Application();

		bool Init() override;
		void Update(float dt) override;
		void Render(float dt) override;

	protected:
		virtual void InitDirect3DInternal() override;

	private:
		std::unique_ptr<SpriteBatch> spriteBatch;
		WRL::ComPtr<ID3D11ShaderResourceView> m_pTexture;
		SimpleMath::Vector2 m_pos;
		//ID3D11ShaderResourceView* m_pTexture;

	};

}

