#include <tchar.h>
#include <string>
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
	//InitalizeSdkObject(m_Manager, m_Scene);
	InitalizeSdkObject();
	if (!m_Manager)
	{
		OutputDebugString("\n nullptr manager \n");
		return E_FAIL;
	}

	int lFileFormat = -1;
	m_Importer = FbxImporter::Create(m_Manager, "");

	if (!m_Manager->GetIOPluginRegistry()->DetectReaderFileFormat(fileName, lFileFormat))
	{
		lFileFormat= m_Manager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	if (!m_Importer || !m_Importer->Initialize(fileName, lFileFormat))
	{
		OutputDebugString("\n error importer \n");
		return E_FAIL;
	}

	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
	m_Manager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);

	// import
	if (!m_Importer || !m_Importer->Import(m_Scene))
	{
		_stprintf_s(DebugStr, 512, _T("■□■ FBXファイルロードエラー. FileName: [ %s ] ■□■\n"), fileName);
		OutputDebugString(DebugStr);
		return E_FAIL;
	}

	FbxAxisSystem OurAxisSystem = FbxAxisSystem::DirectX;
	if (axis == EAxisSystem::OpenGL)
	{
		OurAxisSystem = FbxAxisSystem::OpenGL;
	}

	// DirectX系
	FbxAxisSystem SceneAxisSystem = m_Scene->GetGlobalSettings().GetAxisSystem();
	if (SceneAxisSystem != OurAxisSystem)
	{
		FbxAxisSystem::DirectX.ConvertScene(m_Scene);
	}

	FbxSystemUnit SceneSystemUnit = m_Scene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		// センチメーター単位にコンバートする
		FbxSystemUnit::cm.ConvertScene(m_Scene);
	}

	// 三角形化(三角形以外のデータでもコレで安心)
	TriangulateRecursive(m_Scene->GetRootNode());
	Setup();
	return hr;
}

void FBXLoader::InitalizeSdkObject(FbxManager * pManager, FbxScene * pScene)
{
	pManager = FbxManager::Create();
	if (!pManager)
	{
		OutputDebugString("Error: Unable to create FBX Manager!\n");
	}
	else
	{
		OutputDebugString("\n Success FBX Manager! \n");
		FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());
	}
	FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
	pManager->SetIOSettings(ios);

	FbxString lPath = FbxGetApplicationDirectory();
	pManager->LoadPluginsDirectory(lPath.Buffer());

	pScene = FbxScene::Create(pManager, "My Scene");
	if (!pScene)
	{
		OutputDebugString("Error: Unable to create FBX scene!\n");
	}
	else
	{
		OutputDebugString("\n Success create FBX scene! \n");
	}
}

void FBXLoader::InitalizeSdkObject()
{
	m_Manager = FbxManager::Create();
	if (!m_Manager)
	{
		OutputDebugString("Error: Unable to create FBX Manager!\n");
	}
	else
	{
		OutputDebugString("\n Success FBX Manager! \n");
		FBXSDK_printf("Autodesk FBX SDK version %s\n", m_Manager->GetVersion());
	}
	FbxIOSettings* ios = FbxIOSettings::Create(m_Manager, IOSROOT);
	m_Manager->SetIOSettings(ios);

	FbxString lPath = FbxGetApplicationDirectory();
	m_Manager->LoadPluginsDirectory(lPath.Buffer());

	m_Scene = FbxScene::Create(m_Manager, "My Scene");
	if (!m_Scene)
	{
		OutputDebugString("Error: Unable to create FBX scene!\n");
	}
	else
	{
		OutputDebugString("\n Success create FBX scene! \n");
	}
}

void FBXLoader::TriangulateRecursive(FbxNode * pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch) 
		{
			FbxGeometryConverter lConverter(pNode->GetFbxManager());
			lConverter.Triangulate(m_Scene, true);
		}

		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
		{
			_stprintf_s(DebugStr, 512, _T("■□■ NodeMeshName: [ %s ] ■□■\n"), pNode->GetName());
			OutputDebugString(DebugStr);
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		TriangulateRecursive(pNode->GetChild(lChildIndex));
	}
}

FbxNode& FBXLoader::GetRootNode()
{
	return *m_Scene->GetRootNode();
}

void FBXLoader::Setup()
{
	if (m_Scene->GetRootNode())
	{
		SetupNode(m_Scene->GetRootNode(), "null");
	}
}

void FBXLoader::SetupNode(FbxNode * pNode, std::string parentName)
{
	if (!pNode)
	{
		return;
	}
	FBXMeshNode meshNode;

	meshNode.name = pNode->GetName();
	meshNode.parentName = parentName;
	ZeroMemory(&meshNode.elements, sizeof(MeshElement));

	FbxMesh* lMesh = pNode->GetMesh();

	if (lMesh)
	{
		const int lVertexCount = lMesh->GetControlPointsCount();

		if (lVertexCount > 0)
		{
			// 頂点があるならノードにコピー
			CopyVertexData(lMesh, &meshNode);
		}
	}

	// マテリアル
	const int lMaterialCount = pNode->GetMaterialCount();
	for (int i = 0; i<lMaterialCount; i++)
	{
		FbxSurfaceMaterial* mat = pNode->GetMaterial(i);
		if (!mat)
		{
			continue;
		}
		FBXMaterialNode destMat;
		CopyMatrialData(mat, &destMat);
		meshNode.materialArray.push_back(destMat);
	}

	//
	ComputeNodeMatrix(pNode, &meshNode);
	m_MeshNodeArray.push_back(meshNode);

	const int lCount = pNode->GetChildCount();
	for (int i = 0; i < lCount; i++)
	{
		SetupNode(pNode->GetChild(i), meshNode.name);
	}
}

void FBXLoader::SetFbxColor(FBXMaterialElement & destColor, const FbxDouble3 srcColor)
{
	destColor.a = 1.0f;
	destColor.r = static_cast<float>(srcColor[0]);
	destColor.g = static_cast<float>(srcColor[1]);
	destColor.b = static_cast<float>(srcColor[2]);
}

FbxDouble3 FBXLoader::GetMaterialProperty(const FbxSurfaceMaterial * pMaterial, const char * pPropertyName, const char * pFactorPropertyName, FBXMaterialElement * pElement)
{
	pElement->materialElementType = FBXMaterialElement::MaterialElemementType::ELEMENT_NONE;

	FbxDouble3 lResult(0, 0, 0);
	const FbxProperty lProperty = pMaterial->FindProperty(pPropertyName);
	const FbxProperty lFactorProperty = pMaterial->FindProperty(pFactorPropertyName);
	if (lProperty.IsValid() && lFactorProperty.IsValid())
	{
		lResult = lProperty.Get<FbxDouble3>();
		double lFactor = lFactorProperty.Get<FbxDouble>();
		if (lFactor != 1)
		{
			lResult[0] *= lFactor;
			lResult[1] *= lFactor;
			lResult[2] *= lFactor;
		}

		pElement->materialElementType = FBXMaterialElement::MaterialElemementType::ELEMENT_COLOR;
	}

	if (lProperty.IsValid())
	{
		int existTextureCount = 0;
		const int lTextureCount = lProperty.GetSrcObjectCount<FbxFileTexture>();

		for (int i = 0; i<lTextureCount; i++)
		{
			FbxFileTexture* lFileTexture = lProperty.GetSrcObject<FbxFileTexture>(i);
			if (!lFileTexture)
				continue;

			FbxString uvsetName = lFileTexture->UVSet.Get();
			std::string uvSetString = uvsetName.Buffer();
			std::string filepath = lFileTexture->GetFileName();

			pElement->textureSetArray[uvSetString].push_back(filepath);
			existTextureCount++;
		}

		const int lLayeredTextureCount = lProperty.GetSrcObjectCount<FbxLayeredTexture>();

		for (int i = 0; i<lLayeredTextureCount; i++)
		{
			FbxLayeredTexture* lLayeredTexture = lProperty.GetSrcObject<FbxLayeredTexture>(i);

			const int lTextureFileCount = lLayeredTexture->GetSrcObjectCount<FbxFileTexture>();

			for (int j = 0; j<lTextureFileCount; j++)
			{
				FbxFileTexture* lFileTexture = lLayeredTexture->GetSrcObject<FbxFileTexture>(j);
				if (!lFileTexture)
				{
					continue;
				}

				FbxString uvsetName = lFileTexture->UVSet.Get();
				std::string uvSetString = uvsetName.Buffer();
				std::string filepath = lFileTexture->GetFileName();

				pElement->textureSetArray[uvSetString].push_back(filepath);
				existTextureCount++;
			}
		}

		if (existTextureCount > 0)
		{
			if (pElement->materialElementType == FBXMaterialElement::MaterialElemementType::ELEMENT_COLOR)
			{
				pElement->materialElementType = FBXMaterialElement::MaterialElemementType::ELEMENT_BOTH;
			}
			else
			{
				pElement->materialElementType = FBXMaterialElement::MaterialElemementType::ELEMENT_TEXTURE;
			}
		}
	}

	return lResult;
}

void FBXLoader::CopyMatrialData(FbxSurfaceMaterial * mat, FBXMaterialNode * destMat)
{
	if (!mat)
	{
		return;
	}

	if (mat->GetClassId().Is(FbxSurfaceLambert::ClassId))
	{
		destMat->materialType = FBXMaterialNode::MaterialType::MATERIAL_LAMBERT;
	}
	else if (mat->GetClassId().Is(FbxSurfacePhong::ClassId))
	{
		destMat->materialType = FBXMaterialNode::MaterialType::MATERIAL_PHONG;
	}

	const FbxDouble3 lEmissive = GetMaterialProperty(mat, FbxSurfaceMaterial::sEmissive, FbxSurfaceMaterial::sEmissiveFactor, &destMat->emmisive);
	SetFbxColor(destMat->emmisive, lEmissive);

	const FbxDouble3 lAmbient = GetMaterialProperty(mat, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, &destMat->ambient);
	SetFbxColor(destMat->ambient, lAmbient);

	const FbxDouble3 lDiffuse = GetMaterialProperty(mat,FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, &destMat->diffuse);
	SetFbxColor(destMat->diffuse, lDiffuse);

	const FbxDouble3 lSpecular = GetMaterialProperty(mat, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, &destMat->specular);
	SetFbxColor(destMat->specular, lSpecular);

	//
	FbxProperty lTransparencyFactorProperty = mat->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
	if (lTransparencyFactorProperty.IsValid())
	{
		double lTransparencyFactor = lTransparencyFactorProperty.Get<FbxDouble>();
		destMat->transparencyFactor = static_cast<float>(lTransparencyFactor);
	}

	// Specular Power
	FbxProperty lShininessProperty = mat->FindProperty(FbxSurfaceMaterial::sShininess);
	if (lShininessProperty.IsValid())
	{
		double lShininess = lShininessProperty.Get<FbxDouble>();
		destMat->shininess = static_cast<float>(lShininess);
	}
}

void FBXLoader::ComputeNodeMatrix(FbxNode * pNode, FBXMeshNode * meshNode)
{
	if (!pNode || !meshNode)
	{
		return;
	}

	FbxAnimEvaluator* lEvaluator = m_Scene->GetAnimationEvaluator();
	FbxMatrix lGlobal;
	lGlobal.SetIdentity();

	if (pNode != m_Scene->GetRootNode())
	{
		lGlobal = lEvaluator->GetNodeGlobalTransform(pNode);

		FBXMatrixToFloat16(&lGlobal, meshNode->matrix4x4);
	}
	else
	{
		FBXMatrixToFloat16(&lGlobal, meshNode->matrix4x4);
	}
}

void FBXLoader::CopyVertexData(FbxMesh * pMesh, FBXMeshNode * meshNode)
{
	if (!pMesh)
	{
		return;
	}

	int lPolygonCount = pMesh->GetPolygonCount();

	FbxVector4 pos, nor;

	meshNode->elements.numPosition = 1;
	meshNode->elements.numNormal = 1;

	unsigned int indx = 0;

	for (int i = 0; i<lPolygonCount; i++)
	{
		int lPolygonsize = pMesh->GetPolygonSize(i);

		for (int pol = 0; pol<lPolygonsize; pol++)
		{
			int index = pMesh->GetPolygonVertex(i, pol);
			meshNode->indexArray.push_back(indx);

			pos = pMesh->GetControlPointAt(index);
			pMesh->GetPolygonVertexNormal(i, pol, nor);

			meshNode->positionArray.push_back(pos);
			meshNode->normalArray.push_back(nor);

			++indx;
		}
	}

	FbxStringList uvsetName;
	pMesh->GetUVSetNames(uvsetName);
	int numUVSet = uvsetName.GetCount();
	meshNode->elements.numUVSet = numUVSet;

	bool unmapped = false;

	for (int uv = 0; uv<numUVSet; uv++)
	{
		meshNode->uvsetID[uvsetName.GetStringAt(uv)] = uv;
		for (int i = 0; i<lPolygonCount; i++)
		{
			int lPolygonsize = pMesh->GetPolygonSize(i);

			for (int pol = 0; pol<lPolygonsize; pol++)
			{
				FbxString name = uvsetName.GetStringAt(uv);
				FbxVector2 texCoord;
				pMesh->GetPolygonVertexUV(i, pol, name, texCoord, unmapped);
				meshNode->texcoordArray.push_back(texCoord);
			}
		}
	}

}

FBXMeshNode& FBXLoader::GetNode(const unsigned int id)
{
	return m_MeshNodeArray[id];
}
