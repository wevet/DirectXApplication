#pragma once

#include "d3dUtil.h"
#include "MathHelper.h"

using namespace std;

struct Keyframe
{
	Keyframe();
	~Keyframe();

	float TimePos;
	DirectX::XMFLOAT3 Translation;
	DirectX::XMFLOAT3 Scale;
	DirectX::XMFLOAT4 RotationQuat;

	bool operator == (const Keyframe& key)
	{
		if (Translation.x != key.Translation.x || Translation.y != key.Translation.y || Translation.z != key.Translation.z)
		{
			return false;
		}

		if (Scale.x != key.Scale.x || Scale.y != key.Scale.y || Scale.z != key.Scale.z)
		{
			return false;
		}

		if (RotationQuat.x != key.RotationQuat.x || RotationQuat.y != key.RotationQuat.y || RotationQuat.z != key.RotationQuat.z || RotationQuat.w != key.RotationQuat.w)
		{
			return false;
		}

		return true;
	}

	
};

struct BoneAnimation
{
	float GetStartTime()const;
	float GetEndTime()const;

	void Interpolate(float t, DirectX::XMFLOAT4X4 & M) const;

	vector<Keyframe> Keyframes;
};

struct AnimationClip
{
	float GetClipStartTime()const;
	float GetClipEndTime()const;

	void Interpolate(float t, vector<DirectX::XMFLOAT4X4>& boneTransforms) const;

	vector<BoneAnimation> BoneAnimations;
};

class SkinnedData
{
public:
	UINT BoneCount()const;

	float GetClipStartTime(const string& clipName)const;
	float GetClipEndTime(const string& clipName)const;
	string GetAnimationName(int num) const;
	vector<int> GetBoneHierarchy() const;
	vector<DirectX::XMFLOAT4X4> GetBoneOffsets() const;
	AnimationClip GetAnimation(string clipName) const;
	vector<int> GetSubmeshOffset() const;
	DirectX::XMFLOAT4X4 getBoneOffsets(int num) const;

	void Set(vector<int>& boneHierarchy, vector<DirectX::XMFLOAT4X4>& boneOffsets, unordered_map<string, AnimationClip>& animations);
	void SetAnimation(AnimationClip inAnimation, string inClipName);
	void SetAnimationName(const string& clipName);
	void SetSubmeshOffset(int num);
	void GetFinalTransforms(const string& clipName, float timePos, vector<DirectX::XMFLOAT4X4>& finalTransforms)const;


private:
	vector<int> mBoneHierarchy;
	vector<DirectX::XMFLOAT4X4> mBoneOffsets;
	vector<string> mAnimationName;
	unordered_map<string, AnimationClip> mAnimations;

	vector<int> mSubmeshOffset;
};
