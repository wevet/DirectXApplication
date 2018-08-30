#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#include <fbxsdk.h>
#include <Windows.h>

// UVset名
typedef std::tr1::unordered_map<std::string, int> UVsetID;
// UVSet名, テクスチャパス名(１つのUVSetに複数のテクスチャがぶら下がってることがある)
typedef std::tr1::unordered_map<std::string, std::vector<std::string>> TextureSet;


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

		MaterialElemementType m_MaterialElementType;
		float r, g, b, a;
		TextureSet m_TextureSetArray;

		FBXMaterialElement()
		{
			m_TextureSetArray.clear();
		}

		~FBXMaterialElement()
		{
			Release();
		}

		void Release()
		{
			for (TextureSet::iterator it = m_TextureSetArray.begin(); it != m_TextureSetArray.end(); ++it)
			{
				it->second.clear();
			}
			m_TextureSetArray.clear();
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

		MaterialType m_MaterialType;
		FBXMaterialElement m_Ambient;
		FBXMaterialElement m_Diffuse;
		FBXMaterialElement m_Emmisive;
		FBXMaterialElement m_Specular;

		float m_Shininess;
		float m_TransparencyFactor;
	};

	// mesh構成要素
	struct MeshElement
	{
		// 頂点座標のセットをいくつ持つか
		unsigned int m_NumPosition;
		unsigned int m_NumNormal;
		unsigned int m_NumUVSet; // UVセット数
	};

	// mesh node
	struct FBXMeshNode
	{
		std::string m_Name;
		std::string	m_ParentName;

		MeshElement	m_Elements;
		std::vector<FBXMaterialNode> m_MaterialArray;
		UVsetID	m_UvsetID;

		std::vector<unsigned int> m_IndexArray;
		std::vector<FbxVector4> m_PositionArray;// ポジション配列
		std::vector<FbxVector4>	m_NormalArray;  // 法線配列
		std::vector<FbxVector2>	m_TexcoordArray;// テクスチャ座標配列

		float matrix4x4[16];

		~FBXMeshNode()
		{
			Release();
		}

		void Release()
		{
			m_UvsetID.clear();
			m_TexcoordArray.clear();
			m_MaterialArray.clear();
			m_IndexArray.clear();
			m_PositionArray.clear();
			m_NormalArray.clear();
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
	};


}


