#pragma once
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>
#include <fbxsdk.h>
#include <Windows.h>

// UVset名
// remove namespace tr1
typedef std::unordered_map<std::string, int> UVsetID;
// UVSet名, テクスチャパス名(１つのUVSetに複数のテクスチャがぶら下がってることがある)
typedef std::unordered_map<std::string, std::vector<std::string>> TextureSet;


namespace FBX
{
	// element struct
	struct FBXMaterialElement
	{
		enum MaterialElemementType
		{
			ELEMENT_NONE = 0,
			ELEMENT_COLOR,
			ELEMENT_TEXTURE,
			ELEMENT_BOTH,
			ELEMENT_MAX,
		};

		MaterialElemementType materialElementType;
		float r, g, b, a;
		TextureSet textureSetArray;

		FBXMaterialElement()
		{
			textureSetArray.clear();
		}

		~FBXMaterialElement()
		{
			Release();
		}

		void Release()
		{
			for (TextureSet::iterator it = textureSetArray.begin(); it != textureSetArray.end(); ++it)
			{
				it->second.clear();
			}
			textureSetArray.clear();
		}
	};

	// material node
	struct FBXMaterialNode
	{
		// FBXのマテリアルはLambertとPhongしかない
		enum MaterialType
		{
			MATERIAL_LAMBERT = 0,
			MATERIAL_PHONG,
		};

		MaterialType materialType;
		FBXMaterialElement ambient;
		FBXMaterialElement diffuse;
		FBXMaterialElement emmisive;
		FBXMaterialElement specular;

		float shininess;
		float transparencyFactor;
	};

	// mesh構成要素
	struct MeshElement
	{
		// 頂点座標のセットをいくつ持つか
		unsigned int numPosition;
		unsigned int numNormal;
		unsigned int numUVSet; // UVセット数
	};

	// mesh node
	struct FBXMeshNode
	{
		std::string name;
		std::string	parentName;

		MeshElement	elements;
		std::vector<FBXMaterialNode> materialArray;
		UVsetID	uvsetID;

		std::vector<unsigned int> indexArray;
		std::vector<FbxVector4> positionArray;// ポジション配列
		std::vector<FbxVector4>	normalArray;  // 法線配列
		std::vector<FbxVector2>	texcoordArray;// テクスチャ座標配列

		float matrix4x4[16];

		~FBXMeshNode()
		{
			Release();
		}

		void Release()
		{
			uvsetID.clear();
			texcoordArray.clear();
			materialArray.clear();
			indexArray.clear();
			positionArray.clear();
			normalArray.clear();
		}
	};

	class FBXLoader
	{

	public:
		enum EAxisSystem
		{
			Directx = 0,
			OpenGL,
		};

	protected:
		FbxManager* m_Manager;
		FbxScene* m_Scene;
		FbxImporter* m_Importer;
		FbxAnimLayer* m_AnimLayer;

		std::vector<FBXMeshNode> m_MeshNodeArray;

		void InitalizeSdkObject(FbxManager* pManager, FbxScene* pScene);
		void InitalizeSdkObject();
		void TriangulateRecursive(FbxNode* pNode);

		void SetupNode(FbxNode* pNode, std::string parentName);
		void Setup();

		void CopyVertexData(FbxMesh* pMesh, FBXMeshNode* meshNode);
		void CopyMatrialData(FbxSurfaceMaterial* mat, FBXMaterialNode* destMat);
		void ComputeNodeMatrix(FbxNode* pNode, FBXMeshNode* meshNode);
		void SetFbxColor(FBXMaterialElement& destColor, const FbxDouble3 srcColor);
		FbxDouble3 GetMaterialProperty(const FbxSurfaceMaterial * pMaterial, const char * pPropertyName, const char * pFactorPropertyName, FBXMaterialElement* pElement);

		static void FBXMatrixToFloat16(FbxMatrix* src, float dest[16])
		{
			unsigned int nn = 0;
			for (int i = 0; i<4; i++)
			{
				for (int j = 0; j<4; j++)
				{
					dest[nn] = static_cast<float>(src->Get(i, j));
					nn++;
				}
			}
		}

	public:
		FBXLoader();
		~FBXLoader();

		void Release();

		// 読み込み
		HRESULT LoadFBX(const char* fileName, const EAxisSystem axis);
		FbxNode& GetRootNode();

		// ノード数の取得
		size_t GetNodesCount() { return m_MeshNodeArray.size(); };

		FBXMeshNode& GetNode(const unsigned int id);

	private:
		TCHAR DebugStr[512];

	};


}


