#include <memory>
#include <fstream>

#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOECore/IOEManagerContainer.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOEApplication/IOECommandLine.h>
#include <IOE/IOEMemory/IOEMemory.h>

#include <IOE/IOERenderer/IOERPI.h>
#include <IOE/IOERenderer/IOEShader.h>
#include <IOE/IOERenderer/IOEMaterial.h>
#include <IOE/IOERenderer/IOEModelManager.h>
#include <IOE/IOERenderer/IOEVertexFormat.h>
#include <IOE/IOERenderer/IOEFont.h>

#include <IOE/IOEInput/IOEInputManager.h>
#include <IOE/IOEInput/IOEInputKeys.h>
#include <IOE/IOEInput/IOEInputKeyboard.h>
#include <IOE/IOEInput/IOEInputMouse.h>

#include <IOE/IOEMaths/Matrix.h>
#include <IOE/IOEMaths/Vector.h>

#include "DirectXCollision.h"

#include "ShadowMapping.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////

using namespace IOE;
using namespace IOE::Maths;
using Core::IOEManager;
using Core::IOEManagerContainer;
using Core::g_arrManagers;

//////////////////////////////////////////////////////////////////////////

CubeShadowMap::CubeShadowMap(float fResolution /*=(float)ms_nShadowMapSize*/)
	: m_fShadowMapSize(fResolution)
	, m_tNoBlend(Renderer::IOEBlendState::DefaultNoWrite())
	, m_tDefault(Renderer::IOEBlendState::Default())

{
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::Initialise()
{
	IOEVector vLightPos(
		CTestApplication::GetSingletonPtr()->GetLightPosition());

	m_pBuildShadowmapMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pBuildShadowmapMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials",
		L"BuildCubeShadowMap.ioe.xml"));
	m_pBuildShadowmapMaterial->MapCBuffer("PassSettings");
	m_pBuildShadowmapMaterial->SetShaderVariableValue("PassSettings",
													  "LightPos", vLightPos);
	m_pBuildShadowmapMaterial->UnmapCBuffer("PassSettings");

	m_pShadowMap = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::R32_FLOAT, m_fShadowMapSize,
		m_fShadowMapSize, Renderer::ETextureType::TextureCube);
	m_pShadowMap->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource |
		Renderer::EBufferDescription::RenderTarget);
	m_pShadowMap->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pShadowMap->SetCreateView(false);
	m_pShadowMap->Setup();

	m_pDepthTarget = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::D24_UNORM_S8_UINT, m_fShadowMapSize,
		m_fShadowMapSize);
	m_pDepthTarget->SetBufferDescription(
		Renderer::EBufferDescription::DepthStencil);
	m_pDepthTarget->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pDepthTarget->Setup();

	m_pShaderResources = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::R32_FLOAT, m_pShadowMap->GetWidth(),
		m_pShadowMap->GetHeight(), Renderer::ETextureType::TextureCube);
	m_pShaderResources->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource);
	m_pShaderResources->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pShaderResources->SetCubeFace(Renderer::ECubeFace::All);
	m_pShaderResources->CopyTexture(*m_pShadowMap.get());

	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces;
		 ++nIdx)
	{
		m_arrRenderTargets[nIdx] = std::make_shared<Renderer::IOETexture>(
			Renderer::ETextureFormat::R32_FLOAT, m_pShadowMap->GetWidth(),
			m_pShadowMap->GetHeight(), Renderer::ETextureType::TextureCube);
		m_arrRenderTargets[nIdx]->SetBufferDescription(
			Renderer::EBufferDescription::RenderTarget);
		m_arrRenderTargets[nIdx]->SetBufferAccess(
			Renderer::ECPUAccess::GPUReadWrite);
		m_arrRenderTargets[nIdx]->SetCubeFace((Renderer::ECubeFace)nIdx);
		m_arrRenderTargets[nIdx]->CopyTexture(*m_pShadowMap.get());

		IOEVector vUpVector(
			(nIdx / 2) == 1
				? (nIdx % 2 == 0 ? -g_vForwardVector : g_vForwardVector)
				: g_vUpVector);

		Maths::IOECamera::CCameraProperties tProperties;
		tProperties.eProjectionMethod =
			IOECamera::EProjectionMethod::Perspective;
		tProperties.fFOV	= 90.0f;
		tProperties.fAspect = 1.0f;
		tProperties.fNear   = 0.5f;
		tProperties.fFar	= 1000.0f;

		m_arrCameras[nIdx].SetCameraProperties(tProperties);
		m_arrCameras[nIdx].SetUpVector(vUpVector);

		m_arrVisibilityBounds[nIdx].SetTakeIntoAccountLookAt(true);
	}
	SetLightPosition(vLightPos);

	m_tDefault.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tNoBlend.Setup(Renderer::IOERPI::GetSingletonPtr());
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::PreRender(IOE::Renderer::IOERPI *RPI)
{
	// RPI->SetBlendState(m_tNoBlend);

	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, m_fShadowMapSize, m_fShadowMapSize);
	RPI->SetupViewports();

	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(RPI, m_pBuildShadowmapMaterial);
	}
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::Render(IOE::Renderer::IOERPI *RPI)
{
	IOEVector vLightPos(
		CTestApplication::GetSingletonPtr()->GetLightPosition());

	m_pBuildShadowmapMaterial->MapCBuffer("PassSettings");
	m_pBuildShadowmapMaterial->SetShaderVariableValue("PassSettings",
													  "LightPos", vLightPos);
	m_pBuildShadowmapMaterial->UnmapCBuffer("PassSettings");

	RPI->SetDepthTarget(*m_pDepthTarget.get(), false);

	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces;
		 ++nIdx)
	{
		RPI->SetRenderTarget(*m_arrRenderTargets[nIdx].get());
		RPI->Clear();

		IOEMatrix mViewProj(m_arrCameras[nIdx].GetViewMatrix() *
							m_arrCameras[nIdx].GetProjectionMatrix());

		m_pBuildShadowmapMaterial->MapCBuffer("GlobalSettings");
		m_pBuildShadowmapMaterial->SetShaderVariableValue("GlobalSettings",
														  "View", vLightPos);
		m_pBuildShadowmapMaterial->SetShaderVariableValue(
			"GlobalSettings", "WorldViewProj", mViewProj);
		m_pBuildShadowmapMaterial->UnmapCBuffer("GlobalSettings");

		Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(
			RPI, m_arrVisibilityBounds[nIdx], false);
	}
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::PostRender(IOE::Renderer::IOERPI *RPI)
{
	// RPI->SetBlendState(m_tDefault);
	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer(), false);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer());

	RPI->ResetViewports();
	RPI->SetupViewports();

	RPI->Clear();

	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(
				RPI,
				CTestApplication::GetSingletonPtr()->GetActiveMaterial());
	}

	if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() ==
		EDebugDrawMode::DrawStencilBuffer)
	{
		static float sf_test = 0.0f;
		static int i		 = 0;
		sf_test += Core::g_pEngine->GetFrameDelta().fDelta;
		if (sf_test > 2.5f)
		{
			sf_test -= 2.5f;
			i = (i + 1) % 6;
		}
		CTestApplication::GetSingletonPtr()->CopyFullscreenTarget(
			RPI, m_pShaderResources.get(), nullptr,
			(Renderer::ECubeFace)((int32_t)Renderer::ECubeFace::PositiveX + i),
			0.01f);
	}
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::SetResolution(float fResolution)
{
	m_fShadowMapSize = fResolution;
}

//////////////////////////////////////////////////////////////////////////

void CubeShadowMap::SetLightPosition(const IOE::Maths::IOEVector &vLightPos)
{
	for (int32_t nIdx(0); nIdx < (int32_t)Renderer::ECubeFace::NumFaces;
		 ++nIdx)
	{
		m_arrCameras[nIdx].GetTransform().SetTranslation(vLightPos);
		m_arrCameras[nIdx].SetLookAt(vLightPos + g_arrAxes[nIdx]);

		m_arrCameras[nIdx].UpdateViewMatrices();

		m_arrVisibilityBounds[nIdx].UpdateVisibility(m_arrCameras[nIdx]);
	}
}

//////////////////////////////////////////////////////////////////////////

DualParabaloidShadowMap::DualParabaloidShadowMap(
	float fResolution /*=(float)ms_nShadowMapSize*/)
	: m_tNoBlend(Renderer::IOEBlendState::DefaultNoWrite())
	, m_tDefault(Renderer::IOEBlendState::Default())
	, m_tBackFaceCulling(Renderer::IOERasterizerState::Default())
	, m_tFrontFaceCulling(Renderer::IOERasterizerState::Default())
	, m_fShadowMapSize(fResolution)
{
	m_tFrontFaceCulling.tData.bFrontCounterClockwise =
		!m_tFrontFaceCulling.tData.bFrontCounterClockwise;
}

//////////////////////////////////////////////////////////////////////////

void DualParabaloidShadowMap::Initialise()
{
	IOEVector vLightPos(
		CTestApplication::GetSingletonPtr()->GetLightPosition());

	m_pShadowMap = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::R32_FLOAT, m_fShadowMapSize,
		m_fShadowMapSize, 2);
	m_pShadowMap->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource |
		Renderer::EBufferDescription::RenderTarget);
	m_pShadowMap->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pShadowMap->SetCreateView(false);
	m_pShadowMap->Setup();

	m_pDepthMap = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::D24_UNORM_S8_UINT, m_fShadowMapSize,
		m_fShadowMapSize);
	m_pDepthMap->SetBufferDescription(
		Renderer::EBufferDescription::DepthStencil);
	m_pDepthMap->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pDepthMap->Setup();

	m_pBuildShadowmapMaterial = std::make_shared<Renderer::IOEMaterial>();
	m_pBuildShadowmapMaterial->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials",
		L"BuildParabaloidShadowMap.ioe.xml"));

	m_pCopyBufferMaterial[0] = std::make_shared<Renderer::IOEMaterial>();
	m_pCopyBufferMaterial[0]->AddDefinition("TEXTURE_INDEX", "0");
	m_pCopyBufferMaterial[0]->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"CopyBuffer.ioe.xml"));

	m_pCopyBufferMaterial[1] = std::make_shared<Renderer::IOEMaterial>();
	m_pCopyBufferMaterial[1]->AddDefinition("TEXTURE_INDEX", "1");
	m_pCopyBufferMaterial[1]->LoadMaterial(Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials", L"CopyBuffer.ioe.xml"));

	m_pShaderResources = std::make_shared<Renderer::IOETexture>(
		Renderer::ETextureFormat::R32_FLOAT, m_pShadowMap->GetWidth(),
		m_pShadowMap->GetHeight(), 2);
	m_pShaderResources->SetBufferDescription(
		Renderer::EBufferDescription::ShaderResource);
	m_pShaderResources->SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_pShaderResources->CopyTexture(*m_pShadowMap.get());

	for (int32_t nIdx(0); nIdx < 2; ++nIdx)
	{
		m_arrRenderTargets[nIdx] = std::make_shared<Renderer::IOETexture>(
			Renderer::ETextureFormat::R32_FLOAT, m_pShadowMap->GetWidth(),
			m_pShadowMap->GetHeight(), 2);
		m_arrRenderTargets[nIdx]->SetBufferDescription(
			Renderer::EBufferDescription::RenderTarget);
		m_arrRenderTargets[nIdx]->SetBufferAccess(
			Renderer::ECPUAccess::GPUReadWrite);
		m_arrRenderTargets[nIdx]->SetFirstArrayIndexToMap(nIdx);
		m_arrRenderTargets[nIdx]->SetArrayCountToMap(1);

		m_arrRenderTargets[nIdx]->CopyTexture(*m_pShadowMap.get());
	}

	Maths::IOECamera::CCameraProperties tProperties;
	tProperties.eProjectionMethod = IOECamera::EProjectionMethod::Orthographic;
	tProperties.fFOV			  = 180.0f;
	tProperties.fAspect			  = 1.0f;
	tProperties.fNear			  = 0.5f;
	tProperties.fFar			  = 1000.0f;

	for (uint32_t uIdx(0); uIdx < 2; ++uIdx)
	{
		m_arrCamera[uIdx].SetCameraProperties(tProperties);
	}

	SetLightPosition(vLightPos);

	m_tDefault.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tNoBlend.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tBackFaceCulling.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tFrontFaceCulling.Setup(Renderer::IOERPI::GetSingletonPtr());
}

//////////////////////////////////////////////////////////////////////////

void DualParabaloidShadowMap::PreRender(
	IOE::Renderer::IOERPI *RPI)
{
	RPI->UnsetRenderTarget(0);
	// RPI->SetBlendState(m_tNoBlend);

	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, m_fShadowMapSize, m_fShadowMapSize);
	RPI->SetupViewports();
}

//////////////////////////////////////////////////////////////////////////

void DualParabaloidShadowMap::Render(
	IOE::Renderer::IOERPI *RPI)
{
	IOEVector vLightPos(
		CTestApplication::GetSingletonPtr()->GetLightPosition());
	IOEMatrix mLightInvTranspose(Maths::g_mIdentity.Inverse().Transpose());

	auto pTestApp(CTestApplication::GetSingletonPtr());
	auto pMaterial(m_pBuildShadowmapMaterial);

	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(RPI, m_pBuildShadowmapMaterial);
	}

	RPI->SetDepthTarget(*m_pDepthMap.get(), false);

	for (int32_t nIdx(0); nIdx < 2; ++nIdx)
	{
		if (nIdx == 1)
		{
			RPI->SetRasterizerState(m_tFrontFaceCulling);
		}
		RPI->SetRenderTarget(*m_arrRenderTargets[nIdx].get());

		// Treating the R channel as a depth buffer [0..1], so ensure that
		// we clear to a value of 1.0 so if no fragments are rendered, we
		// ensure
		// that the depth is treated as the far plane and not the near plane.
		RPI->Clear(Maths::ColourList::Red);

		IOEMatrix mViewProj(m_arrCamera[nIdx].GetViewMatrix());

		pMaterial->MapCBuffer("GlobalSettings");
		pMaterial->SetShaderVariableValue("GlobalSettings", "View", vLightPos);
		pMaterial->SetShaderVariableValue("GlobalSettings", "WorldViewProj",
										  mViewProj);
		pMaterial->SetShaderVariableValue(
			"GlobalSettings", "WorldInvTranspose", mLightInvTranspose);
		pMaterial->UnmapCBuffer("GlobalSettings");

		pMaterial->MapCBuffer("PassSettings");
		pMaterial->SetShaderVariableValue("PassSettings", "Near", 0.5f);
		pMaterial->SetShaderVariableValue("PassSettings", "Far", 1000.0f);
		pMaterial->SetShaderVariableValue("PassSettings", "Dir",
										  nIdx == 0 ? 1.0f : -1.0f);
		pMaterial->UnmapCBuffer("PassSettings");

		Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(
			RPI, m_arrVisibilityBounds[nIdx], false);

		if (nIdx == 1)
		{
			RPI->SetRasterizerState(m_tBackFaceCulling);
		}
	}

	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(RPI, pTestApp->GetActiveMaterial());
	}

	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer(), false);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer());
}

//////////////////////////////////////////////////////////////////////////

void DualParabaloidShadowMap::PostRender(
	IOE::Renderer::IOERPI *RPI)
{
	// RPI->SetBlendState(m_tDefault);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer());

	RPI->ResetViewports();
	RPI->SetupViewports();

	RPI->Clear();

	if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() ==
		EDebugDrawMode::DrawStencilBuffer)
	{
		static float sf_test = 0.0f;
		static int i		 = 0;
		sf_test += Core::g_pEngine->GetFrameDelta().fDelta;
		if (sf_test > 2.5f)
		{
			sf_test -= 2.5f;
			i = (i + 1) % 2;
		}
		CTestApplication::GetSingletonPtr()->CopyFullscreenTarget(
			RPI, m_pShaderResources.get(), m_pCopyBufferMaterial[i].get(),
			Renderer::ECubeFace::All, 0.01f);
	}
}

//////////////////////////////////////////////////////////////////////////

void DualParabaloidShadowMap::SetLightPosition(
	const IOE::Maths::IOEVector &vLightPos)
{
	for (std::uint32_t uIdx = 0; uIdx < 2; ++uIdx)
	{
		Maths::IOEVector vForwardVec(uIdx == 0 ? Maths::g_vForwardVector
											   : -Maths::g_vForwardVector);
		m_arrCamera[uIdx].GetTransform().SetTranslation(vLightPos);
		m_arrCamera[uIdx].SetLookAt(vLightPos + Maths::g_vForwardVector);
		m_arrCamera[uIdx].SetUpVector(Maths::g_vUpVector);
		m_arrCamera[uIdx].UpdateViewMatrices();

		m_arrVisibilityBounds[uIdx].UpdateVisibility(vLightPos, vForwardVec);
	}
}

//////////////////////////////////////////////////////////////////////////