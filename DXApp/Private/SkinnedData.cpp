#include "../Public/SkinnedData.h"

using namespace DirectX;
using namespace std;

Keyframe::Keyframe()
	: TimePos(0.0f),
	Translation(0.0f, 0.0f, 0.0f),
	Scale(1.0f, 1.0f, 1.0f),
	RotationQuat(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Keyframe::~Keyframe()
{
}

float BoneAnimation::GetStartTime()const
{
	return Keyframes.front().TimePos;
}

float BoneAnimation::GetEndTime()const
{
	float f = Keyframes.back().TimePos;

	return f;
}

float AnimationClip::GetClipStartTime()const
{
	float t = MathHelper::Infinity;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Min(t, BoneAnimations[i].GetStartTime());
	}

	return t;
}

float AnimationClip::GetClipEndTime()const
{
	float t = 0.0f;
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		t = MathHelper::Max(t, BoneAnimations[i].GetEndTime());
	}
	return t;
}

float SkinnedData::GetClipStartTime(const string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipStartTime();
}

float SkinnedData::GetClipEndTime(const string& clipName)const
{
	auto clip = mAnimations.find(clipName);
	return clip->second.GetClipEndTime();
}

string SkinnedData::GetAnimationName(int num) const
{
	return mAnimationName.at(num);
}

UINT SkinnedData::BoneCount()const
{
	return (UINT)mBoneHierarchy.size();
}

vector<int> SkinnedData::GetBoneHierarchy() const
{
	return mBoneHierarchy;
}

vector<DirectX::XMFLOAT4X4> SkinnedData::GetBoneOffsets() const
{
	return mBoneOffsets;
}

AnimationClip SkinnedData::GetAnimation(string clipName) const
{
	return mAnimations.find(clipName)->second;
}

vector<int> SkinnedData::GetSubmeshOffset() const
{
	return mSubmeshOffset;
}

void BoneAnimation::Interpolate(float t, XMFLOAT4X4& M) const
{
	if (t <= Keyframes.front().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.front().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.front().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.front().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else if (t >= Keyframes.back().TimePos)
	{
		XMVECTOR S = XMLoadFloat3(&Keyframes.back().Scale);
		XMVECTOR P = XMLoadFloat3(&Keyframes.back().Translation);
		XMVECTOR Q = XMLoadFloat4(&Keyframes.back().RotationQuat);

		XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));
	}
	else
	{
		for (UINT i = 0; i < Keyframes.size() - 1; ++i)
		{
			if (t >= Keyframes[i].TimePos && t <= Keyframes[i + 1].TimePos)
			{
				float lerpPercent = (t - Keyframes[i].TimePos) / (Keyframes[i + 1].TimePos - Keyframes[i].TimePos);

				XMVECTOR s0 = XMLoadFloat3(&Keyframes[i].Scale);
				XMVECTOR s1 = XMLoadFloat3(&Keyframes[i + 1].Scale);

				XMVECTOR p0 = XMLoadFloat3(&Keyframes[i].Translation);
				XMVECTOR p1 = XMLoadFloat3(&Keyframes[i + 1].Translation);

				XMVECTOR q0 = XMLoadFloat4(&Keyframes[i].RotationQuat);
				XMVECTOR q1 = XMLoadFloat4(&Keyframes[i + 1].RotationQuat);

				XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
				XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
				XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

				XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
				XMStoreFloat4x4(&M, XMMatrixAffineTransformation(S, zero, Q, P));

				break;
			}
		}
	}
}

void AnimationClip::Interpolate(float t, vector<XMFLOAT4X4>& boneTransforms)const
{
	for (UINT i = 0; i < BoneAnimations.size(); ++i)
	{
		BoneAnimations[i].Interpolate(t, boneTransforms[i]);
	}
}

void SkinnedData::Set(
	vector<int>& boneHierarchy,
	vector<XMFLOAT4X4>& boneOffsets,
	unordered_map<string, AnimationClip>& animations)
{
	mBoneHierarchy = boneHierarchy;
	mBoneOffsets = boneOffsets;
	mAnimations = animations;
}

void SkinnedData::SetAnimation(AnimationClip inAnimation, string ClipName)
{
	mAnimations[ClipName] = inAnimation;
}

void SkinnedData::SetAnimationName(const string & clipName)
{
	mAnimationName.push_back(clipName);
}

void SkinnedData::SetSubmeshOffset(int num)
{
	mSubmeshOffset.push_back(num);
}

void SkinnedData::GetFinalTransforms(const string& clipName, float timePos, vector<XMFLOAT4X4>& finalTransforms)const
{
	UINT numBones = (UINT)mBoneOffsets.size();

	vector<XMFLOAT4X4> toParentTransforms(numBones);

	auto clip = mAnimations.find(clipName);
	clip->second.Interpolate(timePos, toParentTransforms);
	vector<XMFLOAT4X4> toRootTransforms(numBones);

	toRootTransforms[0] = toParentTransforms[0];

	for (UINT i = 1; i < numBones; ++i)
	{
		XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
		int parentIndex = mBoneHierarchy[i];
		XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);
		XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);
		XMStoreFloat4x4(&toRootTransforms[i], toRoot);
	}

	for (UINT i = 0; i < numBones; ++i)
	{
		XMMATRIX offset = XMLoadFloat4x4(&mBoneOffsets[i]);
		XMMATRIX toRoot = XMLoadFloat4x4(&toParentTransforms[i]);
		XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
		finalTransform *= XMMatrixScaling(0.01f, 0.01f, 0.01f);
		XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
	}
}

DirectX::XMFLOAT4X4 SkinnedData::getBoneOffsets(int num) const
{
	return mBoneOffsets.at(num);
}

