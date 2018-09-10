#pragma once

#include <fbxsdk.h>
#include "SkinnedData.h"

using namespace std;

struct BoneIndexAndWeight
{
	BYTE mBoneIndices;
	float mBoneWeight;

	bool operator < (const BoneIndexAndWeight& rhs)
	{
		return (mBoneWeight > rhs.mBoneWeight);
	}
};

struct CtrlPoint
{
	DirectX::XMFLOAT3 mPosition;
	vector<BoneIndexAndWeight> mBoneInfo;
	string mBoneName;

	CtrlPoint()
	{
		mBoneInfo.reserve(4);
	}

	void SortBlendingInfoByWeight()
	{
		sort(mBoneInfo.begin(), mBoneInfo.end());
	}
};

class FbxLoader
{
public:
	FbxLoader();
	~FbxLoader();

	HRESULT LoadFBX(vector<SkinnedVertex>& outVertexVector, vector<uint16_t>& outIndexVector, SkinnedData& outSkinnedData, const string& ClipName, vector<Material>& outMaterial, string fileName);
	HRESULT LoadFBX(vector<Vertex>& outVertexVector, vector<uint16_t>& outIndexVector, vector<Material>& outMaterial, string fileName);
	HRESULT LoadFBX(AnimationClip & animation, const string& ClipName, string fileName);

	bool LoadTXT(vector<SkinnedVertex>& outVertexVector, vector<uint16_t>& outIndexVector, SkinnedData& outSkinnedData, const string& clipName, vector<Material>& outMaterial, string fileName);
	bool LoadTXT(vector<Vertex>& outVertexVector,  vector<uint16_t>& outIndexVector, vector<Material>& outMaterial, string fileName);
	bool LoadAnimation(AnimationClip& animation, const string& clipName, string fileName);
	void GetSkeletonHierarchy(FbxNode * pNode, int curIndex, int parentIndex);
	void GetControlPoints(FbxNode * pFbxRootNode);

	void GetAnimation(FbxScene * pFbxScene, FbxNode * pFbxChildNode, string & outAnimationName, const string& ClipName);
	void GetOnlyAnimation(FbxScene* pFbxScene, FbxNode * pFbxChildNode, AnimationClip& inAnimation);

	void GetVerticesAndIndice(FbxMesh * pMesh, vector<SkinnedVertex>& outVertexVector, vector<uint16_t>& outIndexVector, SkinnedData & outSkinnedData);
	void GetVerticesAndIndice(FbxMesh * pMesh, vector<Vertex>& outVertexVector, vector<uint16_t>& outIndexVector);
	void GetMaterials(FbxNode * pNode, vector<Material>& outMaterial);
	void GetMaterialAttribute(FbxSurfaceMaterial * pMaterial, Material& outMaterial);
	void GetMaterialTexture(FbxSurfaceMaterial * pMaterial, Material & Mat);

	FbxAMatrix GetGeometryTransformation(FbxNode * pNode);


	void ExportAnimation(const AnimationClip& animation, string fileName, const string& clipName);
	void ExportFBX(vector<SkinnedVertex>& outVertexVector, vector<uint16_t>& outIndexVector, SkinnedData& outSkinnedData, const string& clipName, vector<Material>& outMaterial, string fileName);
	void ExportFBX(vector<Vertex>& outVertexVector, vector<uint16_t>& outIndexVector, vector<Material>& outMaterial, string fileName);
	
private:
	unordered_map<unsigned int, CtrlPoint*> mControlPoints;
	unordered_map<string, AnimationClip> mAnimations;

	vector<int> mBoneHierarchy;
	vector<DirectX::XMFLOAT4X4> mBoneOffsets;
	vector<string> mBoneName;
};