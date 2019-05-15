#include <limits>

#include <fbxsdk.h>

#include <IOE/IOECore/IOEManagerContainer.h>
#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOEApplication/IOEApplication.h>

#include <IOE/IOEExceptions/IOEException_Renderer.h>
#include <IOE/IOEExceptions/IOEException_Enum.h>

#include <IOE/IOEMaths/Vector.h>

#include "IOEModelManager.h"
#include "IOEModel.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	using namespace IOE::Core::Algorithm;

	//////////////////////////////////////////////////////////////////////////

	void SetFloat2FromDoublePtr(XMFLOAT2 &rOutput, double *pData)
	{
		rOutput = XMFLOAT2(static_cast<float>(pData[0]),
						   static_cast<float>(pData[1]));
	}

	//////////////////////////////////////////////////////////////////////////

	void SetFloat4FromDoublePtr(XMFLOAT4 &rOutput, double *pData,
								bool /*bInvert*/ = false)
	{
		rOutput = XMFLOAT4(
			static_cast<float>(pData[0]), static_cast<float>(pData[1]),
			static_cast<float>(pData[2]), static_cast<float>(pData[3]));
		/*if (bInvert)
		{
			IOE_ASSERT(IOE::Maths::IOEVector(rOutput).IsNormalised(), "Not
		Normalised");
		}*/
		/*if (bInvert)
		{
			rOutput.x *= 1.0f;
			rOutput.y *= 1.0f;
			rOutput.z *= -1.0f;
		}*/
	}

	//////////////////////////////////////////////////////////////////////////

	IOEModel::IOEModel(const std::wstring &szPath,
					   const std::string &szPassword)
		: m_szPath(szPath)
		, m_szPassword(szPassword)
		, m_tPlatform(*this)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEModel::Load(IOEModelManager *pManager)
	{
		if (!LoadScene(pManager))
			return false;

		// Ensure we're in the right coord system, we default to DirectX
		// settings
		static FbxAxisSystem stOurAxisSystem(FbxAxisSystem::eDirectX);

		FbxAxisSystem tAxisSystem(
			m_pScene->GetGlobalSettings().GetAxisSystem());
		if (tAxisSystem != stOurAxisSystem)
		{
			stOurAxisSystem.ConvertScene(m_pScene);
		}

		FbxNode *pRoot = m_pScene->GetRootNode();

		if (pRoot != nullptr)
		{
			for (std::int32_t nChildIdx(0); nChildIdx < pRoot->GetChildCount();
				 ++nChildIdx)
			{
				if (!LoadNode(pRoot->GetChild(nChildIdx), pManager))
					return false;
			}
		}

		// Setup our platform data
		try
		{
			m_tPlatform.OnSetup(IOE::Renderer::IOERPI::GetSingletonPtr());
		}
		catch (IOE::Exceptions::IOERendererException &)
		{
			return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEModel::LoadScene(IOEModelManager *pModelManager)
	{
		FbxManager *pManager(pModelManager->GetFbxManager());
		m_pScene = FbxScene::Create(pManager, "My Scene");
		if (m_pScene == nullptr)
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create FBX Scene",
										 E_ERROR_RENDERER_UNKNOWN);
		}

		int nAnimStackCount;
		bool bStatus;

		// Create an importer.
		FbxImporter *pImporter = FbxImporter::Create(pManager, "FBXImporter");
		if (pImporter == nullptr)
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create FBX Importer",
										 E_ERROR_RENDERER_UNKNOWN);
		}

		// Initialize the importer by providing a filename.
		const bool bImportStatus =
			pImporter->Initialize(ConvertWideToNarrow(m_szPath).c_str(), -1,
								  pManager->GetIOSettings());

		if (!bImportStatus)
		{
			FbxString error = pImporter->GetStatus().GetErrorString();

			IOE::Core::g_pEngine->GetApplication()->Log(
				"Call to FbxImporter::Initialize() failed.\n");
			IOE::Core::g_pEngine->GetApplication()->Logf(
				"Error returned: %s\n\n", error.Buffer());

			if (pImporter->GetStatus().GetCode() ==
				FbxStatus::eInvalidFileVersion)
			{
				int nFileMajor, nFileMinor, nFileRevision;
				pImporter->GetFileVersion(nFileMajor, nFileMinor,
										  nFileRevision);
				IOE::Core::g_pEngine->GetApplication()->Logf(
					"FBX file format version for file '%s' is %d.%d.%d\n\n",
					m_szPath.c_str(), nFileMajor, nFileMinor, nFileRevision);
			}

			return false;
		}

		if (pImporter->IsFBX())
		{
			// From this point, it is possible to access animation stack
			// information without
			// the expense of loading the entire file.

			IOE::Core::g_pEngine->GetApplication()->Log(
				"Animation Stack Information\n",
				IOE::Application::EVerbosity::Verbose);

			nAnimStackCount = pImporter->GetAnimStackCount();

			IOE::Core::g_pEngine->GetApplication()->Verbosef(
				"    Number of Animation Stacks: %d\n", nAnimStackCount);
			IOE::Core::g_pEngine->GetApplication()->Verbosef(
				"    Current Animation Stack: \"%s\"\n",
				pImporter->GetActiveAnimStackName().Buffer());
			IOE::Core::g_pEngine->GetApplication()->Log(
				"\n", IOE::Application::EVerbosity::Verbose);

			for (int nAnimStack(0); nAnimStack < nAnimStackCount; ++nAnimStack)
			{
				FbxTakeInfo *lTakeInfo = pImporter->GetTakeInfo(nAnimStack);

				IOE::Core::g_pEngine->GetApplication()->Verbosef(
					"    Animation Stack %d\n", nAnimStack);
				IOE::Core::g_pEngine->GetApplication()->Verbosef(
					"         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
				IOE::Core::g_pEngine->GetApplication()->Verbosef(
					"         Description: \"%s\"\n",
					lTakeInfo->mDescription.Buffer());

				// Change the value of the import name if the animation stack
				// should be imported
				// under a different name.
				IOE::Core::g_pEngine->GetApplication()->Verbosef(
					"         Import Name: \"%s\"\n",
					lTakeInfo->mImportName.Buffer());

				// Set the value of the import state to false if the animation
				// stack should be not
				// be imported.
				IOE::Core::g_pEngine->GetApplication()->Verbosef(
					"         Import State: %s\n",
					lTakeInfo->mSelect ? "true" : "false");
				IOE::Core::g_pEngine->GetApplication()->Log(
					"\n", IOE::Application::EVerbosity::Verbose);
			}

			// Set the import states. By default, the import states are always
			// set to
			// true. The code below shows how to change these states.
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_MATERIAL, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_TEXTURE, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_LINK, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_SHAPE, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_GOBO, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_ANIMATION, true);
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_GLOBAL_SETTINGS,
												   true);
		}

		// Import the scene.
		bStatus = pImporter->Import(m_pScene);

		if (bStatus == false &&
			pImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FbxString lString(m_szPassword.c_str());

			pManager->GetIOSettings()->SetStringProp(IMP_FBX_PASSWORD,
													 m_szPassword.c_str());
			pManager->GetIOSettings()->SetBoolProp(IMP_FBX_PASSWORD_ENABLE,
												   true);

			bStatus = pImporter->Import(m_pScene);

			if (bStatus == false &&
				pImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
			{
				IOE::Core::g_pEngine->GetApplication()->Log(
					"\nPassword is wrong, import aborted.\n",
					IOE::Application::EVerbosity::Verbose);
				return false;
			}
		}

		// Destroy the importer.
		pImporter->Destroy();

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEModel::LoadNode(FbxNode *pNode, IOEModelManager *pManager)
	{
		FbxNodeAttribute *pAttribute(pNode->GetNodeAttribute());

		if (pNode->GetNodeAttribute() == nullptr)
		{
			IOE::Core::g_pEngine->GetApplication()->Log(
				"Node attribute null!\n", IOE::Application::EVerbosity::Error);
			return false;
		}

		FbxNodeAttribute::EType eAttributeType =
			pAttribute->GetAttributeType();
		switch (eAttributeType)
		{
		default:
			break;
		/*case FbxNodeAttribute::eMarker:
			DisplayMarker(pNode);
			break;

		case FbxNodeAttribute::eSkeleton:
			DisplaySkeleton(pNode);
			break;*/

		case FbxNodeAttribute::eMesh:
			LoadGeometry(static_cast<FbxMesh *>(pAttribute), pManager);
			break;

			/*case FbxNodeAttribute::eNurbs:
				DisplayNurb(pNode);
				break;

			case FbxNodeAttribute::ePatch:
				DisplayPatch(pNode);
				break;

			case FbxNodeAttribute::eCamera:
				DisplayCamera(pNode);
				break;

			case FbxNodeAttribute::eLight:
				DisplayLight(pNode);
				break;

			case FbxNodeAttribute::eLODGroup:
				DisplayLodGroup(pNode);
				break;*/
		}

		for (std::int32_t nChildIdx(0); nChildIdx < pNode->GetChildCount();
			 ++nChildIdx)
		{
			if (!LoadNode(pNode->GetChild(nChildIdx), pManager))
				return false;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	std::shared_ptr<IOETexture> ImportTexture(const std::wstring &szBasePath,
											  FbxFileTexture *FbxTexture)
	{
		// create an unreal texture asset
		std::wstring szFileName(
			szBasePath +
			IOE::Core::IOEPlatformStatics_Platform::GetPathSplit() +
			ConvertNarrowToWide(FbxTexture->GetRelativeFileName()));

		// Obtain the shortest absolute path
		FixupPath(szFileName);

		std::wstring szExtension(ToLower(GetExtension(szFileName)));
		std::wstring szTexturename(GetFilename(szFileName));

		std::shared_ptr<IOETexture> pPtr;

		try
		{
			pPtr =
				IOETextureManager::GetSingletonPtr()->LoadFromFile(szFileName);
		}
		catch (IOE::Exceptions::IOERendererException &e)
		{
			IOE::Core::g_pEngine->GetApplication()->Errorf(
				"Error opening file '%s', error:\n%s\n", szFileName, e.what());
		}

		return pPtr;
	}

	//////////////////////////////////////////////////////////////////////////

	void ImportTexturesFromNode(
		const std::wstring &szBasePath, FbxNode *Node,
		std::vector<shared_ptr<IOETexture> > &arrOutTextures)
	{
		FbxProperty Property;
		int32_t NbMat = Node->GetMaterialCount();

		// visit all materials
		int32_t MaterialIndex;

		for (MaterialIndex = 0; MaterialIndex < NbMat; MaterialIndex++)
		{
			FbxSurfaceMaterial *Material = Node->GetMaterial(MaterialIndex);

			// go through all the possible textures
			if (Material)
			{
				int32_t TextureIndex;
				FBXSDK_FOR_EACH_TEXTURE(TextureIndex)
				{
					Property = Material->FindProperty(
						FbxLayerElement::sTextureChannelNames[TextureIndex]);

					if (Property.IsValid())
					{
						FbxTexture *lTexture = NULL;

						// Here we have to check if it's layered textures, or
						// just textures:
						int32_t LayeredTextureCount =
							Property.GetSrcObjectCount<FbxLayeredTexture>();
						FbxString PropertyName = Property.GetName();

						EShaderResourceTextureInterpretation eInterpretation(
							EShaderResourceTextureInterpretation::Unknown);

						if (PropertyName == FbxSurfaceMaterial::sNormalMap ||
							PropertyName == FbxSurfaceMaterial::sBump)
						{
							eInterpretation =
								EShaderResourceTextureInterpretation::Bump;
						}
						else if (PropertyName == FbxSurfaceMaterial::sDiffuse)
						{
							eInterpretation =
								EShaderResourceTextureInterpretation::Diffuse;
						}
						if (eInterpretation ==
							EShaderResourceTextureInterpretation::Unknown)
						{
							// We do not support this texture
							continue;
						}
						if (LayeredTextureCount > 0)
						{
							for (int32_t LayerIndex = 0;
								 LayerIndex < LayeredTextureCount;
								 ++LayerIndex)
							{
								FbxLayeredTexture *lLayeredTexture =
									Property.GetSrcObject<FbxLayeredTexture>(
										LayerIndex);
								int32_t NbTextures =
									lLayeredTexture
										->GetSrcObjectCount<FbxTexture>();
								for (int32_t TexIndex = 0;
									 TexIndex < NbTextures; ++TexIndex)
								{
									FbxFileTexture *Texture =
										lLayeredTexture
											->GetSrcObject<FbxFileTexture>(
												TexIndex);
									if (Texture)
									{
										arrOutTextures.push_back(ImportTexture(
											szBasePath, Texture));
										arrOutTextures.back()
											->SetTextureInterpretationType(
												eInterpretation);
									}
								}
							}
						}
						else
						{
							// no layered texture simply get on the property
							int32_t NbTextures =
								Property.GetSrcObjectCount<FbxTexture>();
							for (int32_t TexIndex = 0; TexIndex < NbTextures;
								 ++TexIndex)
							{

								FbxFileTexture *Texture =
									Property.GetSrcObject<FbxFileTexture>(
										TexIndex);
								if (Texture)
								{
									std::shared_ptr<IOETexture> pTexture(
										ImportTexture(szBasePath, Texture));
									if (pTexture != nullptr)
									{
										arrOutTextures.push_back(pTexture);
										arrOutTextures.back()
											->SetTextureInterpretationType(
												eInterpretation);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEModel::LoadGeometry(FbxMesh *pMesh, IOEModelManager *pManager)
	{
		// Remove all bad polygons first
		pMesh->RemoveBadPolygons();
		bool bSuccess(pMesh->GenerateNormals(true, false, false));
		IOE_ASSERT(bSuccess, "Failed to regenerate normals");

		// Get the base layer of the mesh
		FbxLayer *pBaseLayer = pMesh->GetLayer(0);
		if (pBaseLayer == NULL)
		{
			THROW_IOE_RENDERER_EXCEPTION(
				"Failed to find a base layer in the mesh",
				E_ERROR_FAILED_TO_LOAD_MODEL);
		}

		// Obtain a list of all unique UV set names used in the entire model
		std::vector<std::string> arrUVSets;
		for (int32_t nLayerIdx(0); nLayerIdx < pMesh->GetLayerCount();
			 ++nLayerIdx)
		{
			FbxLayer *pLayer(pMesh->GetLayer(nLayerIdx));
			int32_t nUVSetCount(pLayer->GetUVSetCount());

			if (nUVSetCount > 0)
			{
				FbxArray<FbxLayerElementUV const *> EleUVs =
					pLayer->GetUVSets();
				for (int32_t nUVIdx(0); nUVIdx < nUVSetCount; ++nUVIdx)
				{
					FbxLayerElementUV const *ElementUV = EleUVs[nUVIdx];
					if (ElementUV != nullptr)
					{
						std::string szUVSetName(ElementUV->GetName());

						if (std::find(std::begin(arrUVSets),
									  std::end(arrUVSets),
									  szUVSetName) == std::end(arrUVSets))
						{
							arrUVSets.push_back(std::move(szUVSetName));
						}
					}
				}
			}
		}
		int32_t nNumUniqueUVs(static_cast<int32_t>(arrUVSets.size()));

		std::vector<shared_ptr<IOETexture> > arrOutTextures;
		ImportTexturesFromNode(GetBasePath(GetPath()), pMesh->GetNode(),
							   arrOutTextures);
		auto pDefaultBMapTexture(
			IOETextureManager::GetSingletonPtr()->LoadFromFile(
				IOE::Core::Algorithm::JoinPath(
					IOE::Core::g_pEngine->GetMediaPath(), L"Models",
					L"Textures", L"defaultbmap.tga")));
		pDefaultBMapTexture->SetTextureInterpretationType(
			EShaderResourceTextureInterpretation::Bump);

		for (int32_t nSmoothingLayerIdx(0); nSmoothingLayerIdx <
			 pMesh->GetLayerCount(FbxLayerElement::eSmoothing);
			 ++nSmoothingLayerIdx)
		{
			// Computer the smoothing data
			pManager->GetGeometryConverter()
				->ComputePolygonSmoothingFromEdgeSmoothing(pMesh,
														   nSmoothingLayerIdx);
		}

		// Make sure we're using triangle data
		if (!pMesh->IsTriangleMesh())
		{
			IOE::Core::g_pEngine->GetApplication()->Log(
				"Mesh is not triangulated. Attempting to convert now\n",
				IOE::Application::EVerbosity::Verbose);

			FbxNodeAttribute *pTriangularMesh(
				pManager->GetGeometryConverter()->Triangulate(pMesh, true));
			if (pTriangularMesh == nullptr ||
				pTriangularMesh->GetAttributeType() != FbxNodeAttribute::eMesh)
			{
				THROW_IOE_RENDERER_EXCEPTION("Failed to triangulate the mesh",
											 E_ERROR_FAILED_TO_LOAD_MODEL);
			}
			pMesh = pTriangularMesh->GetNode()->GetMesh();
		}

		// Base layer will have changed if we triangulated the mesh
		pBaseLayer = pMesh->GetLayer(0);

		int32_t nNumLayers(pMesh->GetLayerCount());

		// Obtain a flat list of all UV sets in all layers indexed by the
		// unique
		// array (arrUVSets).
		std::vector<FbxLayerElementUV const *> arrUniqueLayerUVs;
		std::vector<FbxLayerElement::EReferenceMode>
			arrUniqueLayerReferenceModes;
		std::vector<FbxLayerElement::EMappingMode> arrUniqueLayerMappingModes;
		if (nNumUniqueUVs > 0)
		{
			arrUniqueLayerUVs.resize(nNumUniqueUVs, nullptr);
			arrUniqueLayerReferenceModes.resize(nNumUniqueUVs,
												FbxLayerElement::eDirect);
			arrUniqueLayerMappingModes.resize(nNumUniqueUVs,
											  FbxLayerElement::eNone);

			for (int32_t nUVIdx(0); nUVIdx < nNumUniqueUVs; ++nUVIdx)
			{
				for (int32_t nUVLayerIdx(0); nUVLayerIdx < nNumLayers;
					 ++nUVLayerIdx)
				{
					FbxLayer *pLayer(pMesh->GetLayer(nUVLayerIdx));
					int32_t nNumUVsThisLayer(pLayer->GetUVSetCount());
					if (nNumUVsThisLayer > 0)
					{
						FbxArray<FbxLayerElementUV const *> arrAllUVsThisLayer(
							pLayer->GetUVSets());
						for (int32_t nUVIdxThisLayer(0);
							 nUVIdxThisLayer < nNumUVsThisLayer;
							 ++nUVIdxThisLayer)
						{
							FbxLayerElementUV const *pUV(
								arrAllUVsThisLayer[nUVIdxThisLayer]);
							if (pUV != nullptr)
							{
								std::string szUVSetName(pUV->GetName());

								if (szUVSetName == arrUVSets[nUVIdx])
								{
									arrUniqueLayerUVs[nUVIdx] = pUV;
									arrUniqueLayerReferenceModes[nUVIdx] =
										pUV->GetReferenceMode();
									arrUniqueLayerMappingModes[nUVIdx] =
										pUV->GetMappingMode();
									break;
								}
							}
						}
					}
				}
			}
		}

		// Get the smoothing group layer
		bool bSmoothingAvailable = false;

		FbxLayerElementSmoothing const *pSmoothingInfo =
			pBaseLayer->GetSmoothing();
		FbxLayerElement::EReferenceMode tSmoothingReferenceMode(
			FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tSmoothingMappingMode(
			FbxLayerElement::eByEdge);
		if (pSmoothingInfo != nullptr)
		{
			if (pSmoothingInfo->GetMappingMode() == FbxLayerElement::eByEdge)
			{
				if (!pManager->GetGeometryConverter()
						 ->ComputePolygonSmoothingFromEdgeSmoothing(pMesh))
				{
					THROW_IOE_RENDERER_EXCEPTION(
						"Failed to compute smoothing data from mesh",
						E_ERROR_FAILED_TO_LOAD_MODEL);
				}
				bSmoothingAvailable = true;
			}

			if (pSmoothingInfo->GetMappingMode() ==
				FbxLayerElement::eByPolygon)
			{
				bSmoothingAvailable = true;
			}

			tSmoothingReferenceMode = pSmoothingInfo->GetReferenceMode();
			tSmoothingMappingMode   = pSmoothingInfo->GetMappingMode();
		}

		// get the first vertex color layer
		FbxLayerElementVertexColor *arrBaseElementVertexColours =
			pBaseLayer->GetVertexColors();
		FbxLayerElement::EReferenceMode tVertexColorReferenceMode(
			FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tVertexColorMappingMode(
			FbxLayerElement::eByControlPoint);
		if (arrBaseElementVertexColours != nullptr)
		{
			tVertexColorReferenceMode =
				arrBaseElementVertexColours->GetReferenceMode();
			tVertexColorMappingMode =
				arrBaseElementVertexColours->GetMappingMode();
		}

		// get the first normal layer
		FbxLayerElementNormal *arrBaseElementNormals =
			pBaseLayer->GetNormals();
		FbxLayerElementTangent *arrBaseElementTangents =
			pBaseLayer->GetTangents();
		FbxLayerElementBinormal *arrBaseElementBinormals =
			pBaseLayer->GetBinormals();

		// whether there is normal, tangent and binormal data in this mesh
		bool bHasNTBInformation = arrBaseElementNormals &&
			arrBaseElementTangents && arrBaseElementBinormals;

		FbxLayerElement::EReferenceMode tNormalReferenceMode(
			FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tNormalMappingMode(
			FbxLayerElement::eByControlPoint);
		if (arrBaseElementNormals != nullptr)
		{
			tNormalReferenceMode = arrBaseElementNormals->GetReferenceMode();
			tNormalMappingMode   = arrBaseElementNormals->GetMappingMode();
		}

		FbxLayerElement::EReferenceMode tTangentReferenceMode(
			FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tTangentMappingMode(
			FbxLayerElement::eByControlPoint);
		if (arrBaseElementTangents != nullptr)
		{
			tTangentReferenceMode = arrBaseElementTangents->GetReferenceMode();
			tTangentMappingMode   = arrBaseElementTangents->GetMappingMode();
		}

		FbxLayerElement::EReferenceMode tBinormalReferenceMode(
			FbxLayerElement::eDirect);
		FbxLayerElement::EMappingMode tBinormalMappingMode(
			FbxLayerElement::eByControlPoint);
		if (arrBaseElementBinormals != nullptr)
		{
			tBinormalReferenceMode =
				arrBaseElementBinormals->GetReferenceMode();
			tBinormalMappingMode = arrBaseElementBinormals->GetMappingMode();
		}

		// Find the amount to offset the indices
		int32_t nPolyOffset(static_cast<int32_t>(m_arrIndices.size()));
		int32_t nVertOffset(static_cast<int32_t>(m_arrVertices.size()));

		std::int32_t nControlPointsCount = pMesh->GetControlPointsCount();
		FbxVector4 *arrControlPoints	 = pMesh->GetControlPoints();

		// Resize vertex data
		m_arrVertices.resize(nVertOffset + nControlPointsCount);

		for (std::int32_t nControlPointIdx = 0;
			 nControlPointIdx < nControlPointsCount; nControlPointIdx++)
		{
			// Load vertices
			double *pData(arrControlPoints[nControlPointIdx]);
			SetFloat4FromDoublePtr(
				m_arrVertices[nVertOffset + nControlPointIdx].m_vPosition,
				pData);
		}

		if (pMesh->GetPolygonCount() == 0)
		{
			THROW_IOE_RENDERER_EXCEPTION("No Polygon data!",
										 E_ERROR_FAILED_TO_LOAD_MODEL);
		}

		// Load all indices
		std::int32_t nVertIdx(0);
		for (int32_t nPolyIdx(0); nPolyIdx < pMesh->GetPolygonCount();
			 ++nPolyIdx)
		{
			if (3 != pMesh->GetPolygonSize(nPolyIdx))
			{
				THROW_IOE_RENDERER_EXCEPTION("Could not triangulate the mesh!",
											 E_ERROR_FAILED_TO_LOAD_MODEL);
			}

			for (int32_t nControlPointIdx(0); nControlPointIdx < 3;
				 ++nControlPointIdx, ++nVertIdx)
			{
				int32_t nDirectVertIdx(nPolyIdx * 3 + nControlPointIdx);

				int32_t nVertMapping(
					pMesh->GetPolygonVertex(nPolyIdx, nControlPointIdx) +
					nVertOffset);
				m_arrIndices.push_back(nVertMapping);

				ModelVertex &rVertex(m_arrVertices[nVertMapping]);

				// Load normals and texture data
				if (arrBaseElementNormals != nullptr)
				{
					// normals may have different reference and mapping mode
					// than tangents and binormals
					int nNormalMapIndex = (tNormalMappingMode ==
										   FbxLayerElement::eByControlPoint)
						? nControlPointIdx
						: nDirectVertIdx;
					int nNormalValueIndex =
						(tNormalReferenceMode == FbxLayerElement::eDirect)
						? nNormalMapIndex
						: arrBaseElementNormals->GetIndexArray().GetAt(
							  nNormalMapIndex);

					SetFloat4FromDoublePtr(
						rVertex.m_vNormal,
						static_cast<double *>(
							arrBaseElementNormals->GetDirectArray().GetAt(
								nNormalMapIndex)),
						true);
					IOE_ASSERT(IOEVector(rVertex.m_vNormal).IsNormalised(),
							   "Normal unnormalized");

					// tangents and binormals share the same reference, mapping
					// mode and index array
					if (bHasNTBInformation)
					{
						int nTangentMapIndex =
							(tTangentMappingMode ==
							 FbxLayerElement::eByControlPoint)
							? nControlPointIdx
							: nDirectVertIdx;
						int nTangentValueIndex =
							(tTangentReferenceMode == FbxLayerElement::eDirect)
							? nTangentMapIndex
							: arrBaseElementTangents->GetIndexArray().GetAt(
								  nTangentMapIndex);

						SetFloat4FromDoublePtr(
							rVertex.m_vTangent,
							static_cast<double *>(
								arrBaseElementTangents->GetDirectArray().GetAt(
									nTangentValueIndex)));

						int nBinormalMapIndex =
							(tBinormalMappingMode ==
							 FbxLayerElement::eByControlPoint)
							? nControlPointIdx
							: nDirectVertIdx;
						int nBinormalValueIndex = (tBinormalReferenceMode ==
												   FbxLayerElement::eDirect)
							? nBinormalMapIndex
							: arrBaseElementBinormals->GetIndexArray().GetAt(
								  nBinormalMapIndex);

						SetFloat4FromDoublePtr(
							rVertex.m_vBinormal,
							static_cast<double *>(
								arrBaseElementBinormals->GetDirectArray()
									.GetAt(nBinormalValueIndex)));
					}
				}
				if (arrBaseElementVertexColours != nullptr)
				{
					int32_t nVertexColorMappingIndex =
						(tVertexColorMappingMode ==
						 FbxLayerElement::eByControlPoint)
						? nVertMapping
						: nDirectVertIdx;

					int32_t nVectorColorIndex =
						(tVertexColorReferenceMode == FbxLayerElement::eDirect)
						? nVertexColorMappingIndex
						: arrBaseElementVertexColours->GetIndexArray().GetAt(
							  nVertexColorMappingIndex);

					SetFloat4FromDoublePtr(
						rVertex.m_vColour,
						reinterpret_cast<double *>(
							&arrBaseElementVertexColours->GetDirectArray()
								 .GetAt(nVectorColorIndex)));
				}
			}
			// Load all UVs in, ensuring we use the unique arrays we preprocess
			// above so all duplicates are ignored.
			for (int32_t nUVLayerIdx(0); nUVLayerIdx < nNumUniqueUVs;
				 ++nUVLayerIdx)
			{
				if (arrUniqueLayerUVs[nUVLayerIdx] != NULL)
				{
					for (int32_t nControlPointIdx(0); nControlPointIdx < 3;
						 ++nControlPointIdx, ++nVertIdx)
					{
						int32_t nVertMapping(pMesh->GetPolygonVertex(
												 nPolyIdx, nControlPointIdx) +
											 nVertOffset);
						int32_t nDirectVertIdx(nPolyIdx * 3 +
											   nControlPointIdx);

						int32_t nUVMapIndex =
							(arrUniqueLayerMappingModes[nUVLayerIdx] ==
							 FbxLayerElement::eByControlPoint)
							? nVertMapping
							: nDirectVertIdx;
						int32_t nUVIndex =
							(arrUniqueLayerReferenceModes[nUVLayerIdx] ==
							 FbxLayerElement::eDirect)
							? nUVMapIndex
							: arrUniqueLayerUVs[nUVLayerIdx]
								  ->GetIndexArray()
								  .GetAt(nUVMapIndex);

						FbxVector2 vUV = arrUniqueLayerUVs[nUVLayerIdx]
											 ->GetDirectArray()
											 .GetAt(nUVIndex);

						SetFloat2FromDoublePtr(
							m_arrVertices[nVertMapping].m_vUV,
							static_cast<double *>(vUV));
					}
				}
			}
		}

		int32_t nEndIdx(static_cast<int32_t>(m_arrIndices.size()));

		m_arrSubsets.emplace_back(nPolyOffset, nEndIdx - nPolyOffset);
		MeshIndex &rIndex(m_arrSubsets.back());
		for (auto pTexture : arrOutTextures)
		{
			rIndex.m_arrTextures.push_back(pTexture);
		}
		if (m_pMaterial)
		{
			rIndex.m_pMaterial =
				std::make_shared<IOEMaterialInstance>(m_pMaterial);
			for (auto pTexture : rIndex.m_arrTextures)
			{
				switch (pTexture->GetTextureInterpretationType())
				{
				case EShaderResourceTextureInterpretation::Bump:
					rIndex.m_pMaterial->AddVariableInstance("BumpTexture",
															pTexture);
					break;
				case EShaderResourceTextureInterpretation::Diffuse:
					rIndex.m_pMaterial->AddVariableInstance("DiffuseTexture",
															pTexture);
					break;
				}
			}
			if (!rIndex.m_pMaterial->HasTextureInstance("BumpTexture"))
			{
				rIndex.m_pMaterial->AddVariableInstance("BumpTexture",
														pDefaultBMapTexture);
			}
		}

		IOEVector vAABBMin(std::numeric_limits<float>::infinity(),
						   std::numeric_limits<float>::infinity(),
						   std::numeric_limits<float>::infinity());
		IOEVector vAABBMax(-std::numeric_limits<float>::infinity(),
						   -std::numeric_limits<float>::infinity(),
						   -std::numeric_limits<float>::infinity());
		for (int32_t nStartIdx(nPolyOffset); nStartIdx < nEndIdx; ++nStartIdx)
		{
			vAABBMin.MinSet(
				m_arrVertices[m_arrIndices[nStartIdx]].m_vPosition);
			vAABBMax.MaxSet(
				m_arrVertices[m_arrIndices[nStartIdx]].m_vPosition);
		}

		rIndex.m_tAABB.Set(vAABBMin, vAABBMax);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModel::SetMaterial(
		IOERPI *pRPI,
		std::shared_ptr<IOE::Renderer::IOEMaterial> pMaterial,
		bool bSetVariables /*=false*/)
	{
		auto pDefaultBMapTexture(
			IOETextureManager::GetSingletonPtr()->LoadFromFile(
				IOE::Core::Algorithm::JoinPath(
					IOE::Core::g_pEngine->GetMediaPath(), L"Models",
					L"Textures", L"defaultbmap.tga")));
		pDefaultBMapTexture->SetTextureInterpretationType(
			EShaderResourceTextureInterpretation::Bump);

		m_pMaterial = pMaterial;

		for (auto &rIndex : m_arrSubsets)
		{
			// if (!rIndex.m_pMaterial || rIndex.m_pMaterial->GetMaterial() !=
			// m_pMaterial.get())
			{
				rIndex.m_pMaterial =
					std::make_shared<IOEMaterialInstance>(m_pMaterial);
				for (auto pTexture : rIndex.m_arrTextures)
				{
					switch (pTexture->GetTextureInterpretationType())
					{
					case EShaderResourceTextureInterpretation::Bump:
						rIndex.m_pMaterial->AddVariableInstance("BumpTexture",
																pTexture);
						break;
					case EShaderResourceTextureInterpretation::Diffuse:
						rIndex.m_pMaterial->AddVariableInstance(
							"DiffuseTexture", pTexture);
						break;
					}
				}
				if (!rIndex.m_pMaterial->HasTextureInstance("BumpTexture"))
				{
					rIndex.m_pMaterial->AddVariableInstance(
						"BumpTexture", pDefaultBMapTexture);
				}
				if (bSetVariables)
				{
					rIndex.m_pMaterial->SetAllVariables(pRPI);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE