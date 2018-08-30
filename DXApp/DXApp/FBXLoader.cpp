#include "FBXLoader.h"


using namespace FBX;


FBXLoader::FBXLoader()
{
	m_Manager = nullptr;
	m_Scene = nullptr;
}


FBXLoader::~FBXLoader()
{
	Release();
}

void FBXLoader::Release()
{
	m_MeshNodeArray.clear();
	if (m_Importer)
	{
		m_Importer->Destroy();
		m_Importer = nullptr;
	}
	if (m_Scene)
	{
		m_Scene->Destroy();
		m_Scene = nullptr;
	}

	if (m_Manager)
	{
		m_Manager->Destroy();
		m_Manager = nullptr;
	}
}

HRESULT FBXLoader::LoadFBX(const char * fileName, const EAxisSystem axis)
{
	if (!fileName)
	{
		return E_FAIL;
	}
	HRESULT hr = S_OK;
	InitalizeSdkObject(m_Manager, m_Scene);
	if (!m_Manager)
	{
		return E_FAIL;
	}

	int lFileFormat = -1;
	m_Importer = FbxImporter::Create(m_Manager, "");

	if (!m_Manager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName, lFileFormat))
	{
		lFileFormat= m_Manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	return hr;
}

FbxNode& FBXLoader::GetRootNode()
{
	// TODO: return ステートメントをここに挿入します
}

FBXMeshNode& FBXLoader::GetNode(const unsigned int id)
{
	// TODO: return ステートメントをここに挿入します
}

void FBXLoader::InitalizeSdkObject(FbxManager * pManager, FbxScene * pScene)
{
}

void FBXLoader::TriangulateRecursive(FbxNode * pNode)
{
}

void FBXLoader::SetupNode(FbxNode * pNode, std::string parentName)
{
}

void FBXLoader::Setup()
{
}

void FBXLoader::CopyVertexData(FbxMesh * pMesh, FBXMeshNode * meshNode)
{
}

void FBXLoader::CopyMatrialData(FbxSurfaceMaterial * mat, FBXMaterialNode * destMat)
{
}

void FBXLoader::ComputeNodeMatrix(FbxNode * pNode, FBXMeshNode * meshNode)
{
}

void FBXLoader::SetFbxColor(FBXMaterialElement & destColor, const FbxDouble3 srcColor)
{
}

FbxDouble3 FBXLoader::GetMaterialProperty(
	const FbxSurfaceMaterial * pMaterial, 
	const char * pPropertyName, 
	const char * pFactorPropertyName, 
	FBXMaterialElement * pElement)
{
	return FbxDouble3();
}
