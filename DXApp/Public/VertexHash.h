#pragma once
#include "FrameResource.h"

namespace std {
	template <>
	struct hash<DirectX::XMFLOAT2>
	{
		std::size_t operator()(const DirectX::XMFLOAT2& k) const
		{
			using std::size_t;
			using std::hash;

			//最初のハッシュ値を計算し、
			//2番目と3番目をXORで結合する
			return ((hash<float>()(k.x)^ (hash<float>()(k.y) << 1)) >> 1);
		}
	};

	template <>
	struct hash<DirectX::XMFLOAT3>
	{
		std::size_t operator()(const DirectX::XMFLOAT3& k) const
		{
			using std::size_t;
			using std::hash;
			return ((hash<float>()(k.x)^ (hash<float>()(k.y) << 1)) >> 1)^ (hash<float>()(k.z) << 1);
		}
	};

	template <>
	struct hash<Vertex>
	{
		std::size_t operator()(const Vertex& k) const
		{
			using std::size_t;
			using std::hash;
			return ((hash<DirectX::XMFLOAT3>()(k.Pos)^ (hash<DirectX::XMFLOAT3>()(k.Normal) << 1)) >> 1)^ (hash<DirectX::XMFLOAT2>()(k.TexC) << 1);
		}
	};
}