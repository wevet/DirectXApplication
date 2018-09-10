#pragma once

#include "../Public/DXApp.h"
#include "../Public/MathHelper.h"
#include "../Public/UploadBuffer.h"
#include "../Public/GeometryGenerator.h"
#include "Camera.h"
#include "FrameResource.h"
#include "FBXLoader.h"
#include "SkinnedData.h"
#include "TextureLoader.h"
#include <tchar.h>

using namespace DirectX::PackedVector;
using namespace std;

const int gNumFrameResources = 3;

struct CollisionSphere
{
	CollisionSphere() = default;

	XMFLOAT3 originVector = { 0.0f, 0.0f, 0.0f };
	float radius = 0.0f;
};

//class PlayerInfo
//{
//private:
//	XMFLOAT3 mPos;
//	XMFLOAT3 mTarget;
//	float mYaw, mRoll;
//	float mRadius;
//	float mPlayerVelocity;
//	UINT score;
//
//public:
//	PlayerInfo() : mPos(0.0f, 2.0f, 0.0f), mTarget(0.0f, 2.0f, 15.0f), mRadius(2.0f), mPlayerVelocity(0.0f), mYaw(0.0f), mRoll(0.0f)
//	{  }
//
//	XMFLOAT3 getPos() const { return mPos; }
//	XMFLOAT3 getTarget() const { return mTarget; }
//	float getYaw() const { return mYaw; }
//	float getRoll() const { return mRoll; }
//	float getRadius() const { return mRadius; }
//	float getVelocity() const { return mPlayerVelocity; }
//
//	void setPos(const XMFLOAT3& pos) { mPos = pos; }
//	void setTarget(const XMFLOAT3& target) { mTarget = target; }
//	void setYaw(const float& yaw) { mYaw = yaw; }
//	void setRoll(const float& roll) { mRoll = roll; }
//	void setRadius(const float& radius) { mRadius = radius; }
//	void setVelocity(const float& velocity) { mPlayerVelocity = velocity; }
//};

struct SkinnedModelInstance
{
	SkinnedData* SkinnedInfo = nullptr;
	vector<DirectX::XMFLOAT4X4> FinalTransforms;
	string ClipName;
	float TimePos = 0.0f;

	//すべてのフレームを呼び出し、時間位置をインクリメントし、
	//現在のアニメーションクリップに基づく各ボーンのアニメーション、および
	//最終的にエフェクトに設定される変換を生成
	//頂点シェーダで処理する。
	void UpdateSkinnedAnimation(float dt)
	{
		TimePos += dt;

		if (TimePos > SkinnedInfo->GetClipEndTime(ClipName))
		{
			TimePos = 0.0f;
		}
		SkinnedInfo->GetFinalTransforms(ClipName, TimePos, FinalTransforms);
	}
};

struct RenderItem
{
	RenderItem() = default;
	RenderItem(const RenderItem& rhs) = delete;
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	//オブジェクトデータが変更されたことを示すDirtyフラグ。定数バッファを更新する必要がある
	//各FrameResourceにオブジェクトcbufferがあるので、
	//各FrameResourceに更新
	int NumFramesDirty = gNumFrameResources;

	//定数バッファのindex
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// Only applicable to skinned render-items.
	UINT SkinnedCBIndex = -1;

	// nullptr if this render-item is not animated by skinned mesh.
	SkinnedModelInstance* SkinnedModelInst = nullptr;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

enum class RenderLayer : int
{
	Opaque = 0,
	SkinnedOpaque,
	Mirrors,
	Reflected,
	Transparent,
	Shadow,
	Count
};

class FBXLoaderApp : public DXApp
{
public:
	FBXLoaderApp(HINSTANCE hInstance);
	FBXLoaderApp(const FBXLoaderApp& rhs) = delete;
	FBXLoaderApp& operator=(const FBXLoaderApp& rhs) = delete;
	~FBXLoaderApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);

	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);
	void UpdateMaterialCB(const GameTimer& gt);
	void UpdateAnimationCBs(const GameTimer & gt);
	void UpdateObjectShadows(const GameTimer & gt);

	void LoadTextures();
	void BuildDescriptorHeaps();
	void BuildTextureBufferViews();
	void BuildConstantBufferViews();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildFbxGeometry();
	void BuildFbxObjectGeometry();
	void BuildMaterials();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildRenderItems();
	void BuildObjectShadows();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const vector<RenderItem*>& ritems);
	array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:
	vector<unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	unordered_map<string, unique_ptr<MeshGeometry>> mGeometries;
	unordered_map<string, unique_ptr<Material>> mMaterials;
	unordered_map<string, unique_ptr<Texture>> mTextures;
	unordered_map<string, ComPtr<ID3DBlob>> mShaders;
	unordered_map<string, ComPtr<ID3D12PipelineState>> mPSOs;

	vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;
	vector<D3D12_INPUT_ELEMENT_DESC> mSkinnedInputLayout;

	// Pass
	PassConstants mMainPassCB;

	// List of all the render items.
	vector<unique_ptr<RenderItem>> mAllRitems;
	vector<unique_ptr<CollisionSphere>> mCollisionRitems;

	// Render items divided by PSO.
	vector<RenderItem*> mRitems[(int)RenderLayer::Count];

	// For FBX
	SkinnedData mSkinnedInfo;
	UINT mSkinnedSrvHeapStart = 0;
	unique_ptr<SkinnedModelInstance> mSkinnedModelInst;

	UINT mObjCbvOffset = 0;
	UINT mPassCbvOffset = 0;
	UINT mMatCbvOffset = 0;
	UINT mSkinCbvOffset = 0;
	UINT mCbvSrvDescriptorSize = 0;

	bool mIsWireframe = false;
	bool mFbxWireframe = false;

	Light mMainLight;
	Camera mCamera;
	POINT mLastMousePos;
};
