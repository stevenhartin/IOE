#include <fstream>
#include <memory>

#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOEApplication/IOECommandLine.h>
#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOECore/IOEManagerContainer.h>
#include <IOE/IOEMemory/IOEMemory.h>

#include <IOE/IOERenderer/IOEFont.h>
#include <IOE/IOERenderer/IOEMaterial.h>
#include <IOE/IOERenderer/IOEModelManager.h>
#include <IOE/IOERenderer/IOERPI.h>
#include <IOE/IOERenderer/IOEShader.h>
#include <IOE/IOERenderer/IOEVertexFormat.h>

#include <IOE/IOEInput/IOEInputKeyboard.h>
#include <IOE/IOEInput/IOEInputKeys.h>
#include <IOE/IOEInput/IOEInputManager.h>
#include <IOE/IOEInput/IOEInputMouse.h>

#include <IOE/IOEMaths/Matrix.h>
#include <IOE/IOEMaths/Vector.h>

#include "DirectXCollision.h"

#include "BidirectionalPathTracing.h"
#include "Main.h"

//////////////////////////////////////////////////////////////////////////

using namespace IOE;
using namespace IOE::Maths;
using Core::g_arrManagers;
using Core::IOEManager;
using Core::IOEManagerContainer;

//////////////////////////////////////////////////////////////////////////

const float BidirectionalPathTracer::ms_fVPLResolution = 256.0f;

//////////////////////////////////////////////////////////////////////////

DebugPerPixelLinkedListVertex::DebugPerPixelLinkedListVertex()
	: vPos(IOE::Maths::g_vIdentity)
{
}

//////////////////////////////////////////////////////////////////////////

DebugPerPixelLinkedListVertex::DebugPerPixelLinkedListVertex(const IOE::Maths::IOEVector &vPos_)
	: vPos(vPos_)
{
}

//////////////////////////////////////////////////////////////////////////

void DebugPerPixelLinkedListVertex::Set(const IOE::Maths::IOEVector &vPos_)
{
	vPos = vPos_;
}

//////////////////////////////////////////////////////////////////////////

BidirectionalPathTracer::ReflectiveShadowMap::ReflectiveShadowMap()
{
}

//////////////////////////////////////////////////////////////////////////

BidirectionalPathTracer::BidirectionalPathTracer()
	: m_tDefaultBlend(Renderer::IOEBlendState::Default())
	, m_tNoColourWrite(Renderer::IOEBlendState::DefaultNoWrite())
	, m_tBackFaceCulling(Renderer::IOERasterizerState::Default())
	, m_tNoCulling(Renderer::IOERasterizerState::Default())
	, m_tVertexBuffer(static_cast<uint32_t>(ms_fVPLResolution * ms_fVPLResolution * 16))
{
	m_tNoCulling.tData.eCullMode = Renderer::ECullMode::None;

	m_tVertexBuffer.SetBufferDescription(IOE::Renderer::EBufferDescription::Vertex);
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::ReflectiveShadowMap::Initialise()
{
	float fResolution(static_cast<float>(ms_nShadowMapResolution));

	tPositionMap.Initialise(fResolution, Renderer::ETextureFormat::R32G32B32A32_FLOAT,
							Renderer::ETextureFormat::R32G32B32A32_FLOAT,
							Renderer::ETextureFormat::R32G32B32A32_FLOAT);

	tDepthMap.Initialise(fResolution, Renderer::ETextureFormat::R24G8_TYPELESS,
						 Renderer::ETextureFormat::R24_UNORM_X8_TYPELESS, Renderer::ETextureFormat::D24_UNORM_S8_UINT,
						 true);

	tNormalMap.Initialise(fResolution, Renderer::ETextureFormat::R8G8B8A8_SNORM,
						  Renderer::ETextureFormat::R8G8B8A8_SNORM, Renderer::ETextureFormat::R8G8B8A8_SNORM);

	tFluxMap.Initialise(fResolution, Renderer::ETextureFormat::R16_UNORM, Renderer::ETextureFormat::R16_UNORM,
						Renderer::ETextureFormat::R16_UNORM);

	tDiffuseMap.Initialise(fResolution, Renderer::ETextureFormat::R8G8B8A8_UNORM,
						   Renderer::ETextureFormat::R8G8B8A8_UNORM, Renderer::ETextureFormat::R8G8B8A8_UNORM);

	tRoughnessMap.Initialise(fResolution, Renderer::ETextureFormat::R16_UNORM, Renderer::ETextureFormat::R16_UNORM,
							 Renderer::ETextureFormat::R16_UNORM);
}

//////////////////////////////////////////////////////////////////////////

template <>
void BidirectionalPathTracer::RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>::Initialise(
	float fResolution, IOE::Renderer::ETextureFormat eTextureFormat, IOE::Renderer::ETextureFormat eResourceFormat,
	IOE::Renderer::ETextureFormat eRenderTargetFormat, bool bDepth /*=false*/)
{
	pTexture = std::make_shared<Renderer::IOETexture>(eTextureFormat, fResolution, fResolution,
													  Renderer::ETextureType::TextureCube);
	pTexture->SetCreateView(false);
	pTexture->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pTexture->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource |
		(bDepth ? Renderer::EBufferDescription::DepthStencil : Renderer::EBufferDescription::RenderTarget));
	pTexture->Setup();

	pResource = std::make_shared<Renderer::IOETexture>(eResourceFormat, fResolution, fResolution,
													   Renderer::ETextureType::TextureCube);
	pResource->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pResource->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	pResource->SetCubeFace(Renderer::ECubeFace::All);
	pResource->CopyTexture(*pTexture.get());

	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces; ++nIdx)
	{
		arrRenderTargets[nIdx] = std::make_shared<Renderer::IOETexture>(eRenderTargetFormat, fResolution, fResolution,
																		Renderer::ETextureType::TextureCube);
		arrRenderTargets[nIdx]->SetBufferDescription(bDepth ? Renderer::EBufferDescription::DepthStencil
															: Renderer::EBufferDescription::RenderTarget);
		arrRenderTargets[nIdx]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		arrRenderTargets[nIdx]->SetCubeFace((Renderer::ECubeFace)nIdx);
		arrRenderTargets[nIdx]->CopyTexture(*pTexture.get());
	}
}

template <>
void BidirectionalPathTracer::RenderableCubemapTexture<2>::Initialise(
	float fResolution, IOE::Renderer::ETextureFormat eTextureFormat, IOE::Renderer::ETextureFormat eResourceFormat,
	IOE::Renderer::ETextureFormat eRenderTargetFormat, bool bDepth /*=false*/)
{
	pTexture = std::make_shared<Renderer::IOETexture>(eTextureFormat, fResolution, fResolution, 2);
	pTexture->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource |
		(bDepth ? Renderer::EBufferDescription::DepthStencil : Renderer::EBufferDescription::RenderTarget));
	pTexture->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pTexture->SetCreateView(false);
	pTexture->Setup();

	pResource =
		std::make_shared<Renderer::IOETexture>(eResourceFormat, pTexture->GetWidth(), pTexture->GetHeight(), 2);
	pResource->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	pResource->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pResource->CopyTexture(*pTexture.get());

	for (int32_t nIdx(0); nIdx < 2; ++nIdx)
	{
		arrRenderTargets[nIdx] = std::make_shared<Renderer::IOETexture>(eRenderTargetFormat, pTexture->GetWidth(),
																		pTexture->GetHeight(), 2);
		arrRenderTargets[nIdx]->SetBufferDescription(bDepth ? Renderer::EBufferDescription::DepthStencil
															: Renderer::EBufferDescription::RenderTarget);
		arrRenderTargets[nIdx]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		arrRenderTargets[nIdx]->SetFirstArrayIndexToMap(nIdx);
		arrRenderTargets[nIdx]->SetArrayCountToMap(1);

		arrRenderTargets[nIdx]->CopyTexture(*pTexture.get());
	}
}

//////////////////////////////////////////////////////////////////////////

template <int TNumFaces, int TNumIndices>
void BidirectionalPathTracer::RenderableCubemapTextureArray<TNumFaces, TNumIndices>::Initialise(
	float fResolution, IOE::Renderer::ETextureFormat eTextureFormat, IOE::Renderer::ETextureFormat eResourceFormat,
	IOE::Renderer::ETextureFormat eRenderTargetFormat, bool bDepth /*=false*/)
{
	pTexture = std::make_shared<Renderer::IOETexture>(eTextureFormat, fResolution, fResolution,
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER_DUAL_PARABALOID_SHADOW_MAPS == 0)
													  Renderer::ETextureType::TextureCubeArray, TNumIndices
#else
													  Renderer::ETextureType::Texture2DArray, TNumIndices * TNumFaces
#endif
	);

	pTexture->SetCreateView(false);
	pTexture->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pTexture->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource |
		(bDepth ? Renderer::EBufferDescription::DepthStencil : Renderer::EBufferDescription::RenderTarget));
	pTexture->Setup();

	pResourceAsArray = std::make_shared<Renderer::IOETexture>(
		eResourceFormat, fResolution, fResolution, pTexture->GetTextureType(), pTexture->GetArrayCountToMap());
	pResourceAsArray->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	pResourceAsArray->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	pResourceAsArray->SetCubeFace(Renderer::ECubeFace::All);
	pResourceAsArray->SetArrayCountToMap(pTexture->GetArrayCountToMap());
	pResourceAsArray->SetFirstArrayIndexToMap(0);

	pResourceAsArray->CopyTexture(*pTexture.get());

	for (int i = 0; i < TNumIndices; ++i)
	{
		pResource[i] = std::make_shared<Renderer::IOETexture>(
			eResourceFormat, fResolution, fResolution, pTexture->GetTextureType(), pTexture->GetArrayCountToMap());
		pResource[i]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		pResource[i]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
		pResource[i]->SetCubeFace(Renderer::ECubeFace::All);
		pResource[i]->SetArrayCountToMap(TNumFaces);
		pResource[i]->SetFirstArrayIndexToMap(i * TNumFaces);

		pResource[i]->CopyTexture(*pTexture.get());

		for (int32_t nIdx(0); nIdx < TNumFaces; ++nIdx)
		{
			arrRenderTargets[i][nIdx] = std::make_shared<Renderer::IOETexture>(
				eRenderTargetFormat, fResolution, fResolution, pTexture->GetTextureType());
			arrRenderTargets[i][nIdx]->SetBufferDescription(bDepth ? Renderer::EBufferDescription::DepthStencil
																   : Renderer::EBufferDescription::RenderTarget);
			arrRenderTargets[i][nIdx]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
			arrRenderTargets[i][nIdx]->SetCubeFace((Renderer::ECubeFace)nIdx);
			arrRenderTargets[i][nIdx]->SetArrayCountToMap(1);
			arrRenderTargets[i][nIdx]->SetFirstArrayIndexToMap(i * TNumFaces + nIdx);
			arrRenderTargets[i][nIdx]->CopyTexture(*pTexture.get());
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::SetLightSettings(const IOEVector &vLightPos, float fIntensity)
{
	if (m_pBuildReflectiveShadowMapMaterial)
	{
		m_pBuildReflectiveShadowMapMaterial->MapCBuffer("RarelyModified");
		m_pBuildReflectiveShadowMapMaterial->SetShaderVariableValue("RarelyModified", "Intensity", fIntensity);
		m_pBuildReflectiveShadowMapMaterial->SetShaderVariableValue(
			"RarelyModified", "Roughness", CTestApplication::GetSingletonPtr()->GetRoughness());
		m_pBuildReflectiveShadowMapMaterial->SetShaderVariableValue(
			"RarelyModified", "Specular", CTestApplication::GetSingletonPtr()->GetSpecular());
		m_pBuildReflectiveShadowMapMaterial->SetShaderVariableValue("RarelyModified", "LightPosition", vLightPos);
		m_pBuildReflectiveShadowMapMaterial->UnmapCBuffer("RarelyModified");
	}
	if (m_pGeneratePerPixelLinkedListMaterial)
	{
		m_pGeneratePerPixelLinkedListMaterial->MapCBuffer("RarelyModified");
		m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue("RarelyModified", "LightIntensity", fIntensity);
		m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue(
			"RarelyModified", "LightPosition", CTestApplication::GetSingletonPtr()->GetLightPosition());
		m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue(
			"RarelyModified", "Roughness", CTestApplication::GetSingletonPtr()->GetRoughness());
		m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue(
			"RarelyModified", "Specular", CTestApplication::GetSingletonPtr()->GetSpecular());
		m_pGeneratePerPixelLinkedListMaterial->UnmapCBuffer("RarelyModified");
	}
	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces; ++nIdx)
	{
		m_arrCubemapCameras[nIdx].GetTransform().SetTranslation(vLightPos);
		m_arrCubemapCameras[nIdx].SetLookAt(vLightPos + g_arrAxes[nIdx]);

		m_arrCubemapCameras[nIdx].UpdateViewMatrices();

		m_arrCubemapVisibility[nIdx].UpdateVisibility(m_arrCubemapCameras[nIdx]);
	}
}

//////////////////////////////////////////////////////////////////////////

IOE::Maths::IOECamera BidirectionalPathTracer::GetVPLCamera()
{
	IOE::Maths::IOECamera tCamera;
	tCamera.SetLookAt(m_vCameraPosition + m_vCameraNormal);
	tCamera.GetTransform().SetTranslation(m_vCameraPosition);

	IOE::Maths::IOECamera::CCameraProperties tProperties;
	tProperties.fFOV = 90.0f;

	tCamera.SetCameraProperties(tProperties);

	return tCamera;
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::Initialise()
{
	m_tShadowMap.Initialise();

	m_arrVPLs[0].Initialise(ms_fVPLResolution, Renderer::ETextureFormat::R32_FLOAT,
							Renderer::ETextureFormat::R32_FLOAT, Renderer::ETextureFormat::R32_FLOAT);

	m_arrVPLs[1].Initialise(ms_fVPLResolution, Renderer::ETextureFormat::R24G8_TYPELESS,
							Renderer::ETextureFormat::R24_UNORM_X8_TYPELESS,
							Renderer::ETextureFormat::D24_UNORM_S8_UINT, true);

	m_pCopyBufferMaterial[0] = std::make_shared<Renderer::IOEMaterial>();
	m_pCopyBufferMaterial[0]->AddDefinition("TEXTURE_INDEX", "0");
	m_pCopyBufferMaterial[0]->LoadMaterial(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"CopyBuffer.ioe.xml"));

	m_pCopyBufferMaterial[1] = std::make_shared<Renderer::IOEMaterial>();
	m_pCopyBufferMaterial[1]->AddDefinition("TEXTURE_INDEX", "1");
	m_pCopyBufferMaterial[1]->LoadMaterial(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"CopyBuffer.ioe.xml"));

	m_pBuildReflectiveShadowMapMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pBuildReflectiveShadowMapMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"GenerateReflectiveShadowMap.ioe.xml"));

	m_pGenerateVPLMaterial = std::make_shared<Renderer::IOEMaterial>();
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER_DUAL_PARABALOID_SHADOW_MAPS == 1)
	m_pGenerateVPLMaterial->LoadMaterial(Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials",
																   L"GenerateVPLDualParabaloid.ioe.xml"));
#else
	m_pGenerateVPLMaterial->LoadMaterial(Core::Algorithm::JoinPat
		Core::g_pEngine->GetMediaPath(), L"Materials", L"GenerateVPLCuboid.ioe.xml"));h(
#endif

	m_pGeneratePerPixelLinkedListMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pGeneratePerPixelLinkedListMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"PerPixelLinkedListRayBundle.ioe.xml"));
	m_pGeneratePerPixelLinkedListMaterial->MapCBuffer("ScreenSizes");
	m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenWidth", ms_fVPLResolution);
	m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenHeight", ms_fVPLResolution);
	m_pGeneratePerPixelLinkedListMaterial->UnmapCBuffer("ScreenSizes");

	m_pClearPerPixelLinkedListMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pClearPerPixelLinkedListMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"ClearPerPixelLinkedListBuffer.ioe.xml"));
	m_pClearPerPixelLinkedListMaterial->MapCBuffer("ScreenSizes");
	m_pClearPerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenWidth", ms_fVPLResolution);
	m_pClearPerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenHeight", ms_fVPLResolution);
	m_pClearPerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "NumSamples", ms_nNumSamples);
	m_pClearPerPixelLinkedListMaterial->UnmapCBuffer("ScreenSizes");

	m_pComputeRadianceMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pComputeRadianceMaterial->LoadMaterial(Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials",
																	   L"CalculateRadianceRayBundle.ioe.xml"));
	m_pComputeRadianceMaterial->MapCBuffer("RarelyModified");
	m_pComputeRadianceMaterial->SetShaderVariableValue("RarelyModified", "ScreenWidth", ms_fVPLResolution);
	m_pComputeRadianceMaterial->SetShaderVariableValue("RarelyModified", "ScreenHeight", ms_fVPLResolution);
	m_pComputeRadianceMaterial->UnmapCBuffer("RarelyModified");

	m_pComputeVPLSampleDataMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pComputeVPLSampleDataMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"ComputeVPLSampleDataMaterial.ioe.xml"));
	m_pComputeVPLSampleDataMaterial->MapCBuffer("RarelyModified");
	m_pComputeVPLSampleDataMaterial->SetShaderVariableValue("RarelyModified", "ScreenWidth", ms_fVPLResolution);
	m_pComputeVPLSampleDataMaterial->SetShaderVariableValue("RarelyModified", "ScreenHeight", ms_fVPLResolution);
	m_pComputeVPLSampleDataMaterial->UnmapCBuffer("RarelyModified");

	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces; ++nIdx)
	{
		IOEVector vUpVector((nIdx / 2) == 1 ? (nIdx % 2 == 0 ? -g_vForwardVector : g_vForwardVector) : g_vUpVector);

		Maths::IOECamera::CCameraProperties tProperties;
		tProperties.eProjectionMethod = IOECamera::EProjectionMethod::Perspective;
		tProperties.fFOV			  = 90.0f;
		tProperties.fAspect			  = 1.0f;
		tProperties.fNear			  = 0.5f;
		tProperties.fFar			  = 1000.0f;

		m_arrCubemapCameras[nIdx].SetCameraProperties(tProperties);
		m_arrCubemapCameras[nIdx].SetUpVector(vUpVector);

		m_arrDefaultAxesCameras[nIdx].SetCameraProperties(tProperties);
		m_arrDefaultAxesCameras[nIdx].SetUpVector(vUpVector);
		m_arrDefaultAxesCameras[nIdx].GetTransform().SetTranslation(Maths::g_vZeroVector);
		m_arrDefaultAxesCameras[nIdx].SetLookAt(g_arrAxes[nIdx]);
		m_arrDefaultAxesCameras[nIdx].UpdateViewMatrices();
	}

	SetLightSettings(CTestApplication::GetSingletonPtr()->GetLightPosition(),
					 CTestApplication::GetSingletonPtr()->GetLightIntensity());

	Maths::IOECamera::CCameraProperties tProperties;
	tProperties.eProjectionMethod = IOECamera::EProjectionMethod::Perspective;
	tProperties.fFOV			  = 180.0f;
	tProperties.fAspect			  = 1.0f;
	tProperties.fNear			  = 0.5f;
	tProperties.fFar			  = 1000.0f;

	m_tForwardCamera.SetCameraProperties(tProperties);
	m_tForwardCamera.GetTransform().SetTranslation(Maths::g_vIdentity);
	m_tForwardCamera.SetLookAt(Maths::g_vForwardVector);
	m_tForwardCamera.SetUpVector(Maths::g_vUpVector);
	m_tForwardCamera.UpdateViewMatrices();

	std::uint32_t uWidth, uHeight;
	IOE::Core::g_pEngine->GetApplication()->GetWindowSize(uWidth, uHeight);

	{
		float fBufferSize(static_cast<float>(m_pGeneratePerPixelLinkedListMaterial->GetStructuredBufferSize(
			"tRWFragmentAndLink"))); // sizeof(float4 + float4 + float +
									 // uint) -> albedo, normal and depth,
									 // next offset
		if (fBufferSize <= 0.0f)
			fBufferSize = 60.0f;
		m_pPixelBuffer[0] = std::make_shared<Renderer::IOETexture>(
			IOE::Renderer::ETextureFormat::Unknown, fBufferSize,
			static_cast<int32_t>(ms_fVPLResolution * ms_fVPLResolution * 8 * ms_nNumSamples));
		m_pPixelBuffer[0]->SetBufferAccess(Renderer::ECPUAccess::GPUWriteBack);
		m_pPixelBuffer[0]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess |
												Renderer::EBufferDescription::ShaderResource);
		m_pPixelBuffer[0]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pPixelBuffer[0]->SetCreateView(false);
		m_pPixelBuffer[0]->SetHasImplicitCounter(true);
		m_pPixelBuffer[0]->Setup();
		m_pPixelBuffer[0]->SetDebugName("PerPixelLinkedList");

		m_pPixelBuffer[1] = std::make_shared<Renderer::IOETexture>(m_pPixelBuffer[0]->GetTextureFormat(), fBufferSize,
																   m_pPixelBuffer[0]->GetArrayLength());
		m_pPixelBuffer[1]->SetTextureType(m_pPixelBuffer[0]->GetTextureType());
		m_pPixelBuffer[1]->SetBufferAccess(m_pPixelBuffer[0]->GetBufferAccess());
		m_pPixelBuffer[1]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess);
		m_pPixelBuffer[1]->SetHasImplicitCounter(m_pPixelBuffer[0]->HasImplicitCounter());
		m_pPixelBuffer[1]->CopyTexture(*m_pPixelBuffer[0]);
		m_pPixelBuffer[1]->SetDebugName("PerPixelLinkedList UAV");

		m_pPixelBuffer[2] = std::make_shared<Renderer::IOETexture>(m_pPixelBuffer[0]->GetTextureFormat(), fBufferSize,
																   m_pPixelBuffer[0]->GetArrayLength());
		m_pPixelBuffer[2]->SetTextureType(m_pPixelBuffer[0]->GetTextureType());
		m_pPixelBuffer[2]->SetBufferAccess(m_pPixelBuffer[0]->GetBufferAccess());
		m_pPixelBuffer[2]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
		m_pPixelBuffer[2]->SetHasImplicitCounter(m_pPixelBuffer[0]->HasImplicitCounter());
		m_pPixelBuffer[2]->CopyTexture(*m_pPixelBuffer[0]);
		m_pPixelBuffer[2]->SetDebugName("PerPixelLinkedList SRV");

		// Pixel Linked list Header
		m_pLinkedListHeadBuffer[0] = std::make_shared<Renderer::IOETexture>(
			IOE::Renderer::ETextureFormat::R32_UINT, static_cast<float>(sizeof(std::uint32_t)),
			static_cast<int32_t>(ms_fVPLResolution * ms_fVPLResolution * ms_nNumSamples));
		m_pLinkedListHeadBuffer[0]->SetBufferAccess(Renderer::ECPUAccess::GPUWriteBack);
		m_pLinkedListHeadBuffer[0]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess |
														 Renderer::EBufferDescription::ShaderResource);
		m_pLinkedListHeadBuffer[0]->SetCreateView(false);
		m_pLinkedListHeadBuffer[0]->SetAllowRawValue(true);
		m_pLinkedListHeadBuffer[0]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pLinkedListHeadBuffer[0]->Setup();
		m_pLinkedListHeadBuffer[0]->SetDebugName("Linked List Head Texture");

		m_pLinkedListHeadBuffer[1] = std::make_shared<Renderer::IOETexture>(
			IOE::Renderer::ETextureFormat::R32_TYPELESS, static_cast<float>(sizeof(std::uint32_t)),
			static_cast<int32_t>(ms_fVPLResolution * ms_fVPLResolution * ms_nNumSamples));
		m_pLinkedListHeadBuffer[1]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pLinkedListHeadBuffer[1]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		m_pLinkedListHeadBuffer[1]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess);
		m_pLinkedListHeadBuffer[1]->SetAllowRawValue(true);
		m_pLinkedListHeadBuffer[1]->CopyTexture(*m_pLinkedListHeadBuffer[0]);
		m_pLinkedListHeadBuffer[1]->SetDebugName("Linked List Head UAV");

		m_pLinkedListHeadBuffer[2] = std::make_shared<Renderer::IOETexture>(
			IOE::Renderer::ETextureFormat::R32_TYPELESS, static_cast<float>(sizeof(std::uint32_t)),
			static_cast<int32_t>(ms_fVPLResolution * ms_fVPLResolution * ms_nNumSamples));
		m_pLinkedListHeadBuffer[2]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pLinkedListHeadBuffer[2]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		m_pLinkedListHeadBuffer[2]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
		m_pLinkedListHeadBuffer[2]->SetAllowRawValue(true);
		m_pLinkedListHeadBuffer[2]->CopyTexture(*m_pLinkedListHeadBuffer[0]);
		m_pLinkedListHeadBuffer[2]->SetDebugName("Linked List Head SRV");
	}

	{
		float fBufferSize(
			static_cast<float>(m_pComputeVPLSampleDataMaterial->GetStructuredBufferSize("OutVPLSampleData")));
		m_pVPLSampleDataBuffer[0] = std::make_shared<Renderer::IOETexture>(IOE::Renderer::ETextureFormat::Unknown,
																		   fBufferSize, ms_nNumSamples);
		// m_pVPLSampleDataBuffer[0]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		m_pVPLSampleDataBuffer[0]->SetBufferAccess(Renderer::ECPUAccess::GPUWriteBack);
		m_pVPLSampleDataBuffer[0]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess |
														Renderer::EBufferDescription::ShaderResource);
		m_pVPLSampleDataBuffer[0]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pVPLSampleDataBuffer[0]->SetCreateView(false);
		m_pVPLSampleDataBuffer[0]->SetHasImplicitCounter(true);
		m_pVPLSampleDataBuffer[0]->Setup();

		m_pVPLSampleDataBuffer[1] = std::make_shared<Renderer::IOETexture>(IOE::Renderer::ETextureFormat::Unknown,
																		   fBufferSize, ms_nNumSamples);
		m_pVPLSampleDataBuffer[1]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		// m_pVPLSampleDataBuffer[1]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		m_pVPLSampleDataBuffer[1]->SetBufferAccess(Renderer::ECPUAccess::GPUWriteBack);
		m_pVPLSampleDataBuffer[1]->SetBufferDescription(Renderer::EBufferDescription::UnorderedAccess);
		m_pVPLSampleDataBuffer[1]->SetHasImplicitCounter(true);
		m_pVPLSampleDataBuffer[1]->CopyTexture(*m_pVPLSampleDataBuffer[0]);

		m_pVPLSampleDataBuffer[2] = std::make_shared<Renderer::IOETexture>(IOE::Renderer::ETextureFormat::Unknown,
																		   fBufferSize, ms_nNumSamples);
		// m_pVPLSampleDataBuffer[2]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
		m_pVPLSampleDataBuffer[2]->SetBufferAccess(Renderer::ECPUAccess::GPUWriteBack);
		m_pVPLSampleDataBuffer[2]->SetTextureType(Renderer::ETextureType::GenericBuffer);
		m_pVPLSampleDataBuffer[2]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
		m_pVPLSampleDataBuffer[2]->SetHasImplicitCounter(true);
		m_pVPLSampleDataBuffer[2]->CopyTexture(*m_pVPLSampleDataBuffer[0]);
	}

	uint32_t uIdx(0);
	for (std::uint32_t uScreenX(0); uScreenX < (uint32_t)ms_fVPLResolution; ++uScreenX)
	{
		float fX(((float)(uScreenX)));
		for (std::uint32_t uScreenY(0); uScreenY < (uint32_t)ms_fVPLResolution; ++uScreenY)
		{
			float fY(((float)(uScreenY)));
			for (std::uint32_t uCollision(0); uCollision < 16; ++uCollision)
			{
				float fZ((float)uCollision);

				m_tVertexBuffer.SetEmplace(uIdx++, IOEVector(fX, fY, fZ));
			}
		}
	}
	m_tVertexBuffer.Setup(IOE::Renderer::IOERPI::GetSingletonPtr());

	m_pDebugPerPixelLinkedListMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pDebugPerPixelLinkedListMaterial->LoadMaterial(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"DebugPerPixelLinkedList.ioe.xml"));
	m_pDebugPerPixelLinkedListMaterial->MapCBuffer("ScreenSizes");
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenWidth", ms_fVPLResolution);
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("ScreenSizes", "ScreenHeight", ms_fVPLResolution);
	m_pDebugPerPixelLinkedListMaterial->UnmapCBuffer("ScreenSizes");

	for (int32_t nPhiIdx(0); nPhiIdx < ms_nNumSamplePhiDirs; ++nPhiIdx)
	{
		float fLongitude = (static_cast<float>(IOE_TWO_PI) / ms_nNumSamplePhiDirs) * nPhiIdx;

		for (int32_t nThetaIdx(0); nThetaIdx < ms_nNumSampleThetaDirs; ++nThetaIdx)
		{
			// + 1 to avoid direct up into the sky
			float fLattitude = (static_cast<float>(IOE_PI) / (ms_nNumSampleThetaDirs + 1)) * (nThetaIdx + 1);

			int32_t nIdx = nPhiIdx * ms_nNumSampleThetaDirs + nThetaIdx;

			/*
			x	=	r cos theta sin phi
			y	=	r sin theta sin phi
			z	=	r cos phi.
			*/
			float fX = Maths::Sin(fLattitude) * Maths::Cos(fLongitude);
			float fY = Maths::Cos(fLattitude);
			float fZ = Maths::Sin(fLattitude) * Maths::Sin(fLongitude);

			Maths::IOEVector vDir(fX, fY, fZ);
			// Maths::IOEVector vDir(0.0f, 0.0f, -1.0f);

			m_arrDirections[nIdx] = vDir;

			Maths::IOEAABB tAABB(CTestApplication::GetSingletonPtr()->GetMainModel()->GetAABB());
			Maths::IOEMatrix mTransform(Maths::IOEMatrix::LookAt(Maths::g_vIdentity, -vDir));
			Maths::IOEAABB tAABBTransformed = tAABB.Transform(mTransform);

			Maths::IOEVector vMin(tAABBTransformed.m_vBounds[0]);
			Maths::IOEVector vMax(tAABBTransformed.m_vBounds[1]);
			Maths::IOEVector vCentre(vMin + (vMax - vMin) * 0.5f);
			vCentre.SetW(1.0f);

			float fMaxBounds((vMax - vMin).MaxElement() * 1.5f);

			// vCentre = vMin + IOEVector(fMaxBounds*0.5f, fMaxBounds*0.5f,
			// fMaxBounds*0.5f);
			Maths::IOECamera &rCamera(m_arrCamerasGlobalRayBundles[nIdx]);

			Maths::IOEVector vTrans(vCentre + vDir * 45.0f);
			Maths::IOERay tRay(vTrans, -vDir * fMaxBounds);
			float fDistance(0.0f);

			if (tAABB.Intersect(tRay, &fDistance))
			{
				vTrans = vCentre + vDir * (fDistance + 50.0f);
			}

			vTrans.SetW(1.0f);
			rCamera.GetTransform() = IOETransform(g_mIdentity);
			rCamera.GetTransform().SetTranslation(vTrans);
			rCamera.SetLookAt(vCentre);
			rCamera.SetUpVector(Maths::IOEVector(0.0f, 1.0f, 0.0f));
			rCamera.SetOrthographic(fMaxBounds, fMaxBounds, 1.0f, fMaxBounds * 2.0f);
			rCamera.UpdateViewMatrices();

			Maths::IOEMatrix mViewProj((rCamera.GetViewMatrix() * rCamera.GetProjectionMatrix()));
			for (int i = 0; i < 16; ++i)
			{
				m_arrRayBundleLocations[nIdx][i] = mViewProj.GetCell(i % 4, i / 4);
			}
			vTrans.Get(m_arrRayBundleLocations[nIdx][16], m_arrRayBundleLocations[nIdx][17],
					   m_arrRayBundleLocations[nIdx][18], m_arrRayBundleLocations[nIdx][19]);
			vDir.Get(m_arrRayBundleLocations[nIdx][20], m_arrRayBundleLocations[nIdx][21],
					 m_arrRayBundleLocations[nIdx][22], m_arrRayBundleLocations[nIdx][23]);
		}
	}

	Renderer::IOETextureSource tSource;
	tSource.pSrc			= &m_arrRayBundleLocations[0][0];
	tSource.uNumMipMaps		= 1;
	tSource.uPixelSizeBytes = 0;

	// float fBufferSize(sizeof(float) * 16);
	float fBufferSize(
		static_cast<float>(CTestApplication::GetSingletonPtr()->GetActiveMaterial()->GetStructuredBufferSize(
			"InRayBundleCameraLocations")));
	if (fBufferSize <= 0.0f)
		fBufferSize = sizeof(float) * 24;
	m_pRayBundleCameraLocationBuffer = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::Unknown, fBufferSize, static_cast<int32_t>(ms_nNumSamples));
	m_pRayBundleCameraLocationBuffer->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pRayBundleCameraLocationBuffer->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	m_pRayBundleCameraLocationBuffer->SetTextureType(Renderer::ETextureType::GenericBuffer);
	m_pRayBundleCameraLocationBuffer->Setup(&tSource);

	//////////////////////////////////////////////////////////////////////////
	// Global ray bundle render targets
	m_pGlobalRayBundleRenderTarget[0] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::R32G32B32A32_FLOAT, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleRenderTarget[0]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleRenderTarget[0]->SetBufferDescription(Renderer::EBufferDescription::RenderTarget |
															Renderer::EBufferDescription::ShaderResource);
	m_pGlobalRayBundleRenderTarget[0]->SetCreateView(false);
	m_pGlobalRayBundleRenderTarget[0]->Setup();

	m_pGlobalRayBundleRenderTarget[1] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::R32G32B32A32_FLOAT, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleRenderTarget[1]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleRenderTarget[1]->SetBufferDescription(Renderer::EBufferDescription::RenderTarget);
	m_pGlobalRayBundleRenderTarget[1]->CopyTexture(*m_pGlobalRayBundleRenderTarget[0]);

	m_pGlobalRayBundleRenderTarget[2] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::R32G32B32A32_FLOAT, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleRenderTarget[2]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleRenderTarget[2]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	m_pGlobalRayBundleRenderTarget[2]->CopyTexture(*m_pGlobalRayBundleRenderTarget[0]);

	//////////////////////////////////////////////////////////////////////////
	// Global ray bundle depth targets
	m_pGlobalRayBundleDepthTarget[0] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::R24G8_TYPELESS, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleDepthTarget[0]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleDepthTarget[0]->SetBufferDescription(Renderer::EBufferDescription::DepthStencil |
														   Renderer::EBufferDescription::ShaderResource);
	m_pGlobalRayBundleDepthTarget[0]->SetCreateView(false);
	m_pGlobalRayBundleDepthTarget[0]->Setup();

	m_pGlobalRayBundleDepthTarget[1] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::D24_UNORM_S8_UINT, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleDepthTarget[1]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleDepthTarget[1]->SetBufferDescription(Renderer::EBufferDescription::DepthStencil);
	m_pGlobalRayBundleDepthTarget[1]->CopyTexture(*m_pGlobalRayBundleDepthTarget[0].get());

	m_pGlobalRayBundleDepthTarget[2] = std::make_shared<Renderer::IOETexture>(
		IOE::Renderer::ETextureFormat::R24_UNORM_X8_TYPELESS, ms_fVPLResolution, ms_fVPLResolution);
	m_pGlobalRayBundleDepthTarget[2]->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pGlobalRayBundleDepthTarget[2]->SetBufferDescription(Renderer::EBufferDescription::ShaderResource);
	m_pGlobalRayBundleDepthTarget[2]->CopyTexture(*m_pGlobalRayBundleDepthTarget[0].get());

	m_tDefaultBlend.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tNoColourWrite.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tBackFaceCulling.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tNoCulling.Setup(Renderer::IOERPI::GetSingletonPtr());
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::PreRender(Renderer::IOERPI *RPI)
{
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::Render(Renderer::IOERPI *RPI)
{
	START_PROFILE_MARKER("Creating Reflective Shadow Map", Maths::ColourList::Green3);
	CreateReflectiveShadowMap(RPI);
	END_PROFILE_MARKER();

	START_PROFILE_MARKER("Creating VPL Sample Data", Maths::ColourList::Yellow3);
	CreateVPLSampleData(RPI);
	END_PROFILE_MARKER();

	START_PROFILE_MARKER("Generating VPLS", Maths::ColourList::Orange3);
	GenerateVPLs(RPI);
	END_PROFILE_MARKER();

	START_PROFILE_MARKER("Create Per Pixel Linked List", Maths::ColourList::Red3);
	CreatePerPixelLinkedList(RPI);
	END_PROFILE_MARKER();
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::CreateReflectiveShadowMap(Renderer::IOERPI *RPI)
{
	// Assign materials on all models to the build octree materials

	for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx)->SetMaterial(RPI,
																				  m_pBuildReflectiveShadowMapMaterial);
	}

	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, (float)BidirectionalPathTracer::ReflectiveShadowMap::ms_nShadowMapResolution,
					 (float)BidirectionalPathTracer::ReflectiveShadowMap::ms_nShadowMapResolution);
	RPI->SetupViewports();

	IOEVector vLightPos(CTestApplication::GetSingletonPtr()->GetLightPosition());
	IOEMatrix mLightInvTranspose(Maths::g_mIdentity.Inverse().Transpose());

	auto pMaterial(m_pBuildReflectiveShadowMapMaterial);

	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces; ++nIdx)
	{
		RPI->SetRenderTarget(*m_tShadowMap.tPositionMap.arrRenderTargets[nIdx], 0, false);
		RPI->SetRenderTarget(*m_tShadowMap.tNormalMap.arrRenderTargets[nIdx], 1, false);
		RPI->SetRenderTarget(*m_tShadowMap.tFluxMap.arrRenderTargets[nIdx], 2, false);
		RPI->SetRenderTarget(*m_tShadowMap.tDiffuseMap.arrRenderTargets[nIdx], 3, false);
		RPI->SetRenderTarget(*m_tShadowMap.tRoughnessMap.arrRenderTargets[nIdx], 4, false);
		RPI->SetDepthTarget(*m_tShadowMap.tDepthMap.arrRenderTargets[nIdx], true);

		RPI->Clear();

		IOEMatrix mViewProj(m_arrCubemapCameras[nIdx].GetViewMatrix() *
							m_arrCubemapCameras[nIdx].GetProjectionMatrix());

		pMaterial->MapCBuffer("GlobalSettings");
		pMaterial->SetShaderVariableValue("GlobalSettings", "WorldViewProj", mViewProj);
		pMaterial->UnmapCBuffer("GlobalSettings");

		pMaterial->Set(RPI, false);

		/*IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(RPI,
			m_arrCubemapVisibility[nIdx], false);*/
		IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(RPI, false);

		RPI->UnsetRenderTarget(4);
		RPI->UnsetRenderTarget(3);
		RPI->UnsetRenderTarget(2);
		RPI->UnsetRenderTarget(1);
		RPI->UnsetRenderTarget(0);
		RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer());
	}

	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer(), 0);
	RPI->ResetViewports();
	RPI->SetupViewports();

	// Assign materials on all models to the build octree materials
	for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx)->SetMaterial(
			RPI, CTestApplication::GetSingletonPtr()->GetActiveMaterial());
	}
}

//////////////////////////////////////////////////////////////////////////

struct VPLSampleData_Struct
{
	float Position[3];
	float Normal[3];
	float Diffuse[4];
	float Roughness;
	float Intensity;
};

void BidirectionalPathTracer::CreateVPLSampleData(IOE::Renderer::IOERPI *RPI)
{
	RPI->ResetUAVs();
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "OutVPLSampleData", m_pVPLSampleDataBuffer[1].get());

	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Position",
														m_tShadowMap.tPositionMap.pResource.get());
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Normal",
														m_tShadowMap.tNormalMap.pResource.get());
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Intensity",
														m_tShadowMap.tFluxMap.pResource.get());
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Diffuse",
														m_tShadowMap.tDiffuseMap.pResource.get());
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Roughness",
														m_tShadowMap.tRoughnessMap.pResource.get());

	// TODO: make all RayDir in a single buffer so this can be called in a
	// single dispatch call rather than
	// ms_nNumSamples calls.
	m_pComputeVPLSampleDataMaterial->Set(RPI, false);
	for (int32_t nSample(0); nSample < ms_nNumSamples; ++nSample)
	{
		IOEVector vForward(m_arrDirections[nSample]);
		vForward.NormaliseSet();
		m_pComputeVPLSampleDataMaterial->MapCBuffer("Globals");
		m_pComputeVPLSampleDataMaterial->SetShaderVariableValue("Globals", "RayDir", vForward);
		m_pComputeVPLSampleDataMaterial->SetShaderVariableValue("Globals", "CurrentSample", nSample);
		m_pComputeVPLSampleDataMaterial->UnmapCBuffer("Globals");

#if !defined(IOE_FINAL)
		RPI->AddDebugLine(CTestApplication::GetSingletonPtr()->GetLightPosition(),
						  CTestApplication::GetSingletonPtr()->GetLightPosition() + vForward * 100.0f);
#endif

		m_pComputeVPLSampleDataMaterial->Dispatch(RPI, 1, 1, 1);
	}
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Position", nullptr);
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Normal", nullptr);
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Intensity", nullptr);
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Diffuse", nullptr);
	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "InReflectiveShadowMap_Roughness", nullptr);

	m_pComputeVPLSampleDataMaterial->SetTextureVariable(RPI, "OutVPLSampleData", nullptr);

#if 1
	void *pData;
	if (m_pVPLSampleDataBuffer[0]->MapBuffer(Renderer::EBufferMapType::ReadOnly, pData, 0))
	{
		VPLSampleData_Struct *pFormattedData(reinterpret_cast<VPLSampleData_Struct *>(pData));

		for (int32_t nSample(0); nSample < ms_nNumSamples; ++nSample)
		{
			IOEVector vPosition(pFormattedData[nSample].Position[0], pFormattedData[nSample].Position[1],
								pFormattedData[nSample].Position[2]);
			IOEVector vNormal(pFormattedData[nSample].Normal[0], pFormattedData[nSample].Normal[1],
							  pFormattedData[nSample].Normal[2]);

			static int32_t s_nNeededIdx(0);
			if (nSample == s_nNeededIdx)
			{
				m_vCameraPosition = vPosition;
				m_vCameraNormal   = vNormal;
			}

#if !defined(IOE_FINAL)
			RPI->AddDebugBox(vPosition, Maths::g_vOneVector * 0.1f, Maths::ColourList::Cyan);
			RPI->AddDebugLine(vPosition, vPosition + vNormal, Maths::ColourList::Purple, Maths::ColourList::Purple);
#endif
		}

		m_pVPLSampleDataBuffer[0]->UnmapBuffer(0);
	}
#endif
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::GenerateVPLs(Renderer::IOERPI *RPI)
{
#
	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, ms_fVPLResolution, ms_fVPLResolution);
	RPI->SetupViewports();

	// Assign materials on all models to the build octree materials
	for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx)->SetMaterial(RPI, m_pGenerateVPLMaterial);
	}

	for (int32_t nIdx(0); nIdx < ms_nNumSamples; ++nIdx)
	{
		char buffer[64];
		sprintf_s<64>(buffer, "Generating VPL %d", nIdx);

		::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(
			buffer, ::IOE::Renderer::EProfileMarkerType::Start, Maths::ColourList::White);

		float s_fLongitude = ((static_cast<float>(IOE_TWO_PI) / (ms_nNumSampleThetaDirs + 2)) *
							  ((nIdx % (ms_nNumSampleThetaDirs)) + 1));
		float s_fLattitude =
			((static_cast<float>(IOE_PI) / (ms_nNumSamplePhiDirs + 2)) * ((nIdx / ms_nNumSamplePhiDirs) + 1));

		// Core::g_pEngine->GetApplication()->Logf("Long: %.2f, Lat: %.2f\n",
		// s_fLongitude, s_fLattitude);

		/*
		x	=	r cos theta sin phi
		y	=	r sin theta sin phi
		z	=	r cos phi.
		*/
		float fX = Maths::Cos(s_fLongitude) * Maths::Sin(s_fLattitude);
		float fY = Maths::Cos(s_fLattitude);
		float fZ = Maths::Sin(s_fLongitude) * Maths::Sin(s_fLattitude);

		Maths::IOEVector vDir(fX, fY, fZ);

		m_pGenerateVPLMaterial->MapCBuffer("PassSettings");
		m_pGenerateVPLMaterial->SetShaderVariableValue("PassSettings", "SampleDir", vDir);
		m_pGenerateVPLMaterial->SetShaderVariableValue("PassSettings", "SampleIndex", nIdx);

		m_pGenerateVPLMaterial->SetShaderVariableValue("PassSettings", "Near", 0.5f);
		m_pGenerateVPLMaterial->SetShaderVariableValue("PassSettings", "Far", 1000.0f);

		m_pGenerateVPLMaterial->UnmapCBuffer("PassSettings");

		m_pGenerateVPLMaterial->SetTextureVariable(RPI, "InVPLSampleData", m_pVPLSampleDataBuffer[2].get());

		for (int32_t eDir(0); eDir < m_arrVPLs[0].ms_nNumFaces; ++eDir)
		{
			char buffer[64];
			sprintf_s<64>(buffer, "Direction %d", eDir);

			::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(
				buffer, ::IOE::Renderer::EProfileMarkerType::Start, Maths::ColourList::White);

			RPI->SetDepthTarget(*m_arrVPLs[1].arrRenderTargets[nIdx][eDir], false);
			RPI->SetRenderTarget(*m_arrVPLs[0].arrRenderTargets[nIdx][eDir]);

			m_pGenerateVPLMaterial->MapCBuffer("GlobalSettings");
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER_DUAL_PARABALOID_SHADOW_MAPS == 1)
			IOEMatrix mWVP(Maths::g_mIdentity * m_tForwardCamera.GetViewMatrix() *
						   m_tForwardCamera.GetProjectionMatrix());
#else
			IOEMatrix mWVP(Maths::g_mIdentity * m_arrDefaultAxesCameras[eDir].GetViewMatrix() *
						   m_arrDefaultAxesCameras[eDir].GetProjectionMatrix());
#endif
			m_pGenerateVPLMaterial->SetShaderVariableValue("GlobalSettings", "ViewProj", mWVP);
			m_pGenerateVPLMaterial->UnmapCBuffer("GlobalSettings");

			RPI->Clear(IOE::Maths::ColourList::Black.WithW(0.0f));

			IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(RPI, false);

			END_PROFILE_MARKER();
		}

		m_pGenerateVPLMaterial->SetTextureVariable(RPI, "InVPLSampleData", nullptr);

		END_PROFILE_MARKER();
	}
	RPI->ResetViewports();
	RPI->SetupViewports();

	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer(), 0, false);
	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer());

	// Assign materials on all models to the build octree materials
	for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx)->SetMaterial(
			RPI, CTestApplication::GetSingletonPtr()->GetActiveMaterial());
	}
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::CreatePerPixelLinkedList(IOE::Renderer::IOERPI *RPI)
{
	RPI->ResetUAVs();
	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, ms_fVPLResolution, ms_fVPLResolution);
	RPI->SetupViewports();

	// Disable depth write
	// RPI->UnsetRenderTarget();
	RPI->SetDepthTarget(*m_pGlobalRayBundleDepthTarget[1].get(), false);
	RPI->SetRenderTarget(*m_pGlobalRayBundleRenderTarget[1].get(), 0, true);

	// Disable culling and colour writes
	RPI->SetRasterizerState(m_tNoCulling);
	RPI->SetBlendState(m_tDefaultBlend);

	/*m_pClearPerPixelLinkedListMaterial->SetTextureVariable("tRWFragmentAndLink",
	m_pPixelBuffer[1]);
	m_pClearPerPixelLinkedListMaterial->Dispatch(RPI.get(),
		static_cast<int32_t>(ms_fVPLResolution),
		static_cast<int32_t>(ms_fVPLResolution), 1);
	m_pClearPerPixelLinkedListMaterial->SetTextureVariable("tRWFragmentAndLink",
	nullptr);*/

	// Assign materials on all models to the build octree materials
	for (int32_t nSample(0); nSample < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nSample)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nSample)->SetMaterial(
			RPI, m_pGeneratePerPixelLinkedListMaterial);
	}

	RPI->ClearUAV(m_pLinkedListHeadBuffer[1].get(), 0xFFFFFFFF);

	m_pGeneratePerPixelLinkedListMaterial->SetTextureVariable(RPI, "tRWFragmentListHead",
															  m_pLinkedListHeadBuffer[1].get());
	m_pGeneratePerPixelLinkedListMaterial->SetTextureVariable(RPI, "tRWFragmentAndLink", m_pPixelBuffer[1].get());

	for (uint32_t nSample(0); nSample < ms_nNumSamples; ++nSample)
	{
		char buffer[64];
		sprintf_s<64>(buffer, "Global Ray Bundle %d", nSample);

		::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(
			buffer, ::IOE::Renderer::EProfileMarkerType::Start, Maths::ColourList::White);

		m_pGeneratePerPixelLinkedListMaterial->MapCBuffer("PassSettings");
		m_pGeneratePerPixelLinkedListMaterial->SetShaderVariableValue("PassSettings", "CurrentSample", nSample);
		m_pGeneratePerPixelLinkedListMaterial->UnmapCBuffer("PassSettings");
		RPI->SetCamera(m_arrCamerasGlobalRayBundles[nSample]);

		RPI->Clear();
		IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(RPI);

		RPI->SetCamera(RPI->GetDefaultCamera());

		END_PROFILE_MARKER();
	}

	m_pGeneratePerPixelLinkedListMaterial->SetTextureVariable(RPI, "tRWFragmentAndLink", nullptr);
	m_pGeneratePerPixelLinkedListMaterial->SetTextureVariable(RPI, "tRWFragmentListHead", nullptr);

#if 0
	void *pData = nullptr;
	m_pPixelBuffer[0].get()->MapBuffer(IOE::Renderer::EBufferMapType::ReadOnly,
									   pData, 0);
	m_pPixelBuffer[0].get()->UnmapBuffer(0);

	struct _Data
	{
		float Albedo[4];
		float Normal[4];
		float Radiance[4];
		float Depth;
		float Roughness;
		uint32_t Next;
	};

	_Data *pSrcData = (_Data *)pData;

	void *pData2 = nullptr;
	m_pLinkedListHeadBuffer[0].get()->MapBuffer(IOE::Renderer::EBufferMapType::ReadOnly, pData2, 0);
	m_pLinkedListHeadBuffer[0].get()->UnmapBuffer(0);

	std::uint32_t *pSrcData2 = (std::uint32_t *)(pData2);

	struct Colour
	{
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
			};
			uint8_t _[3];
		};
		float dist;

		Colour(float dist_, float x, float y, float z)
			: dist(dist_)
			, r((uint8_t)(x * 255.0f))
			, g((uint8_t)(y * 255.0f))
			, b((uint8_t)(z * 255.0f))
		{
		}

		Colour(const Colour &rhs)
			: r(rhs.r)
			, g(rhs.g)
			, b(rhs.b)
			, dist(rhs.dist)
		{
		}

		Colour(Colour &&rhs)
			: r(rhs.r)
			, g(rhs.g)
			, b(rhs.b)
			, dist(rhs.dist)
		{
		}

		Colour &operator=(const Colour &rhs)
		{
			r	= rhs.r;
			g	= rhs.g;
			b	= rhs.b;
			dist = rhs.dist;
			return *this;
		}

		Colour &operator=(Colour &&rhs)
		{
			r	= rhs.r;
			g	= rhs.g;
			b	= rhs.b;
			dist = rhs.dist;
			return *this;
		}

		inline bool operator<(const Colour &rhs) const
		{
			return dist < rhs.dist;
		}
	};

	auto fn = [](int x, int y, _Data *pdata, std::uint32_t *pSrcPixel,
				 std::uint32_t nIdx, std::int32_t nSample) {
		char buffer[16];
		sprintf_s<16>(buffer, "test%02d_%02d.bmp", nSample, nIdx);
		std::ofstream out(buffer, std::ios::out | std::ios::binary);

		BITMAPFILEHEADER tWBFH;
		tWBFH.bfType	  = 0x4d42;
		tWBFH.bfSize	  = 14 + 40 + (x * y * 3);
		tWBFH.bfReserved1 = 0;
		tWBFH.bfReserved2 = 0;
		tWBFH.bfOffBits   = 14 + 40;
		out.write((char *)&tWBFH, 14);

		BITMAPINFOHEADER tW2BH;
		memset(&tW2BH, 0, 40);
		tW2BH.biSize		= 40;
		tW2BH.biWidth		= x;
		tW2BH.biHeight		= y;
		tW2BH.biPlanes		= 1;
		tW2BH.biBitCount	= 24;
		tW2BH.biCompression = 0;
		out.write((char *)&tW2BH, 40);

		std::vector<Colour> arrColours;

		for (int i = y - 1; i >= 0; --i)
		{
			for (int j = 0; j < x; ++j)
			{
				std::uint32_t uIdx = pSrcPixel[nSample * x * y + i * x + j];
				while (uIdx != ~0U)
				{
					_Data &rSource = pdata[uIdx];
					uIdx		   = rSource.Next;
					arrColours.emplace_back(rSource.Depth, rSource.Albedo[0],
											rSource.Albedo[1],
											rSource.Albedo[2]);
				}
				std::sort(std::begin(arrColours), std::end(arrColours),
						  std::less<Colour>());

				if (arrColours.size() <= nIdx)
				{
					out << (uint8_t)0xFF;
					out << (uint8_t)0xFF;
					out << (uint8_t)0xFF;
				}
				else
				{
					out << arrColours[nIdx].r;
					out << arrColours[nIdx].g;
					out << arrColours[nIdx].b;
				}
				arrColours.clear();
			}
		}
		out.close();
	};
	static bool bTest = false;
	if (!bTest)
	{
		for (int i = 0; i < ms_nNumSamples; ++i)
		{
			fn((int)ms_fVPLResolution, (int)ms_fVPLResolution, pSrcData,
			   pSrcData2, 0, i);
			fn((int)ms_fVPLResolution, (int)ms_fVPLResolution, pSrcData,
			   pSrcData2, 1, i);
			fn((int)ms_fVPLResolution, (int)ms_fVPLResolution, pSrcData,
			   pSrcData2, 2, i);
			fn((int)ms_fVPLResolution, (int)ms_fVPLResolution, pSrcData,
			   pSrcData2, 3, i);
		}
		bTest = true;
	}
#else

	m_pComputeRadianceMaterial->SetTextureVariable(RPI, "InOutFragmentAndLink", m_pPixelBuffer[1].get());
	m_pComputeRadianceMaterial->SetTextureVariable(RPI, "InFragmentListHead", m_pLinkedListHeadBuffer[2].get());

	m_pComputeRadianceMaterial->Set(RPI, false);
	for (int32_t nSample(0); nSample < ms_nNumSamples; ++nSample)
	{
		IOEVector vForward(m_arrCamerasGlobalRayBundles[nSample].GetViewMatrix().GetColumn(2));
		vForward.NormaliseSet();
		m_pComputeRadianceMaterial->MapCBuffer("Globals");
		m_pComputeRadianceMaterial->SetShaderVariableValue("Globals", "RayDir", vForward);
		m_pComputeRadianceMaterial->SetShaderVariableValue("Globals", "CurrentSample", nSample);
		m_pComputeRadianceMaterial->UnmapCBuffer("Globals");

		m_pComputeRadianceMaterial->Dispatch(RPI, static_cast<uint32_t>(ms_fVPLResolution),
											 static_cast<uint32_t>(ms_fVPLResolution), 1);
	}

	m_pComputeRadianceMaterial->SetTextureVariable(RPI, "InOutFragmentAndLink", nullptr);
	m_pComputeRadianceMaterial->SetTextureVariable(RPI, "InFragmentListHead", nullptr);
#endif

	// Assign materials on all models to the build octree materials
	for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx)->SetMaterial(
			RPI, CTestApplication::GetSingletonPtr()->GetActiveMaterial());
	}

	// Revert to default rasterise and blend states
	RPI->SetRasterizerState(m_tBackFaceCulling);
	RPI->SetBlendState(m_tDefaultBlend);

	// Disable depth write
	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer(), false);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer());

	RPI->ResetViewports();
	RPI->SetupViewports();
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::PostRender(Renderer::IOERPI *RPI)
{
	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer(), false);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer(), 0, true);

	RPI->Clear();
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::FinishRender(Renderer::IOERPI *RPI)
{
	IOE::Renderer::IOEMaterial *pMaterial(nullptr);
	float fOutputScale(1.0f);
	Renderer::IOETexture *pTexture(nullptr);
	if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMPosition)
	{
		pTexture = m_tShadowMap.tPositionMap.pResource.get();
		IOEAABB tAABB(CTestApplication::GetSingletonPtr()->GetMainModel()->GetAABB());

		IOEVector vOctreeScale = g_vOneVector / (tAABB.m_vBounds[1] - tAABB.m_vBounds[0]);
		float fX, fY, fZ;
		vOctreeScale.Get(fX, fY, fZ);

		fOutputScale = Maths::Min(fX, Maths::Min(fY, fZ));
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMNormal)
	{
		pTexture = m_tShadowMap.tNormalMap.pResource.get();
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMFlux)
	{
		pTexture = m_tShadowMap.tFluxMap.pResource.get();
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMDepth)
	{
		pTexture = m_tShadowMap.tDepthMap.pResource.get();
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMRoughness)
	{
		pTexture = m_tShadowMap.tRoughnessMap.pResource.get();
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawRSMDiffuse)
	{
		pTexture = m_tShadowMap.tDiffuseMap.pResource.get();
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawVPLDepth ||
			 CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawVPLFlux)
	{
		static float sf_test = 0.0f;
		static int i		 = 0;
		sf_test += Core::g_pEngine->GetFrameDelta().fDelta;

		if (sf_test >= 2.5f)
		{
			i = (i + 1) % ms_nNumSamples;
			sf_test -= 2.5f;
		}
		int32_t nIdx(CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawVPLFlux ? 0 : 1);
		pTexture	 = m_arrVPLs[nIdx].pResource[i].get();
		pMaterial	= m_pCopyBufferMaterial[0].get();
		fOutputScale = 100.0f;
	}
	else if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() == EDebugDrawMode::DrawPerPixelLinkedList)
	{
		DebugDrawScene(RPI);
	}
	if (pTexture != nullptr)
	{
		static float sf_test = 0.0f;
		static int i		 = 0;
		/*sf_test += Core::g_pEngine->GetFrameDelta().fDelta;
		if (sf_test > 2.5f)
		{
			sf_test -= 2.5f;
			i = (i + 1) % 6;
		}*/
		CTestApplication::GetSingletonPtr()->CopyFullscreenTarget(
			RPI, pTexture, pMaterial, (Renderer::ECubeFace)((int32_t)Renderer::ECubeFace::PositiveX + i),
			fOutputScale);
	}
}

//////////////////////////////////////////////////////////////////////////

void BidirectionalPathTracer::DebugDrawScene(Renderer::IOERPI *RPI)
{
	Input::IOEInputKeyboard *pKeyboard(
		IOE::Input::IOEInputManager::GetSingletonPtr()->FindDevice<IOE::Input::IOEInputKeyboard>());
	static int nIdx(0);
	if (pKeyboard->JustReleased(IOE::Input::EInputKey::M))
	{
		nIdx = (nIdx + 1) % ms_nNumSamples;
	}

	m_pDebugPerPixelLinkedListMaterial->Set(RPI);

	m_pDebugPerPixelLinkedListMaterial->SetTextureVariable(RPI, "InOutFragmentAndLink", GetPixelLinkedListBuffer());
	m_pDebugPerPixelLinkedListMaterial->SetTextureVariable(RPI, "InFragmentListHead", GetHeadBuffer(), -1);

	IOEMatrix mWVP(m_arrRayBundleLocations[0][0], m_arrRayBundleLocations[0][1], m_arrRayBundleLocations[0][2],
				   m_arrRayBundleLocations[0][3], m_arrRayBundleLocations[0][4], m_arrRayBundleLocations[0][5],
				   m_arrRayBundleLocations[0][6], m_arrRayBundleLocations[0][7], m_arrRayBundleLocations[0][8],
				   m_arrRayBundleLocations[0][9], m_arrRayBundleLocations[0][10], m_arrRayBundleLocations[0][11],
				   m_arrRayBundleLocations[0][12], m_arrRayBundleLocations[0][13], m_arrRayBundleLocations[0][14],
				   m_arrRayBundleLocations[0][15]);
	IOEVector vFwd(m_arrRayBundleLocations[0][20], m_arrRayBundleLocations[0][21], m_arrRayBundleLocations[0][22],
				   m_arrRayBundleLocations[0][23]);
	IOEQuaternion qRot(vFwd, Maths::g_vUpVector);

	Maths::IOECamera &rCamera(RPI->GetCamera());
	m_pDebugPerPixelLinkedListMaterial->MapCBuffer("Globals");
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue(
		"Globals", "WorldViewProj", rCamera.GetViewMatrix() * rCamera.GetProjectionMatrix());
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("Globals", "RayBundleXFormInv",
															   qRot.ToRotationMatrix());
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue(
		"Globals", "RayBundleOrigin",
		IOEVector(m_arrRayBundleLocations[0][16], m_arrRayBundleLocations[0][17], m_arrRayBundleLocations[0][18],
				  m_arrRayBundleLocations[0][19]));
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("Globals", "RayBundleDirection", vFwd);
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("Globals", "SampleIndex", nIdx);
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("Globals", "ScreenWidth", ms_fVPLResolution);
	m_pDebugPerPixelLinkedListMaterial->SetShaderVariableValue("Globals", "ScreenHeight", ms_fVPLResolution);
	m_pDebugPerPixelLinkedListMaterial->UnmapCBuffer("Globals");

	RPI->DrawBuffer(&m_tVertexBuffer, Renderer::EPrimitiveTopology::Point);

	m_pDebugPerPixelLinkedListMaterial->SetTextureVariable(RPI, "InFragmentListHead", nullptr);
	m_pDebugPerPixelLinkedListMaterial->SetTextureVariable(RPI, "InOutFragmentAndLink", nullptr);

	std::vector<std::tuple<int32_t, int32_t> > arrPoints;
	CTestApplication *pApplication  = static_cast<CTestApplication *>(IOE::Core::g_pEngine->GetApplication());
	IOE::Renderer::IOEModel *pModel = pApplication->GetMainModel();
	for (auto &rInput : pModel->GetVertexBuffer())
	{
		IOEVector vInput = rInput.m_vPosition;
		vInput.SetW(1.0f);
		vInput = m_arrCamerasGlobalRayBundles[0].GetViewMatrix() *
			m_arrCamerasGlobalRayBundles[0].GetProjectionMatrix() * vInput;
		float fX   = (vInput.GetX() + 0.5f) * 0.5f * ms_fVPLResolution;
		float fY   = (1.0f - ((vInput.GetY() + 1.0f) * 0.5f)) * ms_fVPLResolution;
		int32_t nX = (int32_t)(fX);
		int32_t nY = (int32_t)(fY);
		arrPoints.emplace_back(nX, nY);
	}
	int i = 0, j = i;
	i = j;
	arrPoints;
}

/////////////////////////////////////////////////////////////////////////