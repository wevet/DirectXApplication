#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"
#include "UploadBuffer.h"

using namespace DirectX;

struct ObjectConstants
{
    XMFLOAT4X4 World = MathHelper::Identity4x4();
	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
};

struct PassConstants
{
	XMFLOAT4X4 View     = MathHelper::Identity4x4();
    XMFLOAT4X4 InvView  = MathHelper::Identity4x4();
    XMFLOAT4X4 Proj     = MathHelper::Identity4x4();
    XMFLOAT4X4 InvProj  = MathHelper::Identity4x4();
    XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;
	XMFLOAT4 AmbientLight = { 0.0f, 0.0f, 0.0f, 1.0f };

	Light Lights[MaxLights];
};

struct SkinnedConstants
{
	XMFLOAT4X4 BoneTransforms[96];
};

struct Vertex
{
    XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;

	bool operator==(const Vertex& other) const
	{
		if (Pos.x != other.Pos.x || Pos.y != other.Pos.y || Pos.z != other.Pos.z)
		{
			return false;
		}

		if (Normal.x != other.Normal.x || Normal.y != other.Normal.y || Normal.z != other.Normal.z)
		{
			return false;
		}

		if (TexC.x != other.TexC.x || TexC.y != other.TexC.y)
		{
			return false;
		}
		return true;
	}
};

struct SkinnedVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 TexC;
	XMFLOAT3 BoneWeights;
	BYTE BoneIndices[4];
	
	uint16_t MaterialIndex;
};

struct FrameResource
{
public:
    
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount, UINT skinnedObjectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CmdListAlloc;
    std::unique_ptr<UploadBuffer<PassConstants>> PassCB = nullptr;
    std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	std::unique_ptr<UploadBuffer<MaterialConstants>>  MaterialCB = nullptr;
	std::unique_ptr<UploadBuffer<SkinnedConstants>> SkinnedCB = nullptr;
    UINT64 Fence = 0;
};
