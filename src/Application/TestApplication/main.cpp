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

#include "main.h"

using namespace IOE;
using namespace IOE::Maths;
using Core::g_arrManagers;
using Core::IOEManager;
using Core::IOEManagerContainer;

using std::shared_ptr;

// Create the platform entry point and link it to the class
CREATE_PLATFORM_ENTRY(CTestApplication)

//////////////////////////////////////////////////////////////////////////

CTestApplication::CTestApplication(const IOE::Application::C_CommandLine &rCommandLine)
	: IOEApplication(rCommandLine)
	, m_fIntensity(25.0f)
	, m_fRoughness(0.5f)
	, m_tFullscreenBuffer(6)
	, m_tNoClipRasterizerState(Renderer::IOERasterizerState::Default())
	, m_tDefaultRasterizerState(Renderer::IOERasterizerState::Default())
	, m_vLightPos(5.77300262f, 1.90854383f, 3.73790574f)
	, m_fLightSpecular(0.04f)
	, m_fLightRadius(50.0f)
	, m_vLightColour(Maths::ColourList::White)
	, m_eDebugDrawMode(EDebugDrawMode::None)
	, m_pClampSampler(std::make_shared<Renderer::IOESamplerState>(Renderer::IOESamplerState::Default()))
	, m_pDefaultSampler(std::make_shared<Renderer::IOESamplerState>(Renderer::IOESamplerState::Default()))
	, m_bCameraDirty(true)
	, m_eCameraMode(ECameraMode::Camera)
	, m_vCameraPosition(-10.0f, 1.5f, 5.0f)
	, m_qCameraRotation(20.0f, 110.0f, 0.0f)
	, m_dRenderTime(0.0)
	, m_nNumnRenders(0)
	, m_fLastRender(0.0f)
	, m_szDebugMode("None")
	, m_szLightingMode("Direct And Indirect")
	, m_nLightingMode(0)
	, m_nCurrentSerialisedDataIdx(-1)
{
	m_tFullscreenBuffer.SetBufferDescription(Renderer::EBufferDescription::Vertex);
	m_tNoClipRasterizerState.tData.eCullMode		= Renderer::ECullMode::None;
	m_tNoClipRasterizerState.tData.bDepthClipEnable = false;

	m_pClampSampler->tData.eAddressModeU = Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.eAddressModeV = Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.eAddressModeW = Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.vBorderColour = Maths::ColourList::Black.WithW(0.0f);
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::CreateApplication()
{
	// Create all managers used in this application here
	g_arrManagers.CreateManager<Renderer::IOERPI>();
	g_arrManagers.CreateManager<Renderer::IOETextureManager>();
	g_arrManagers.CreateManager<Renderer::IOEVertexFormatContainer>();
	g_arrManagers.CreateManager<Renderer::IOETextManager>();
	g_arrManagers.CreateManager<Input::IOEInputManager>();

	IOEApplication::CreateApplication();
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::OnManagerPostInit()
{
	IOEApplication::OnManagerPostInit();

	//////////////////////////////////////////////////////////////////////////
	// Custom vertex layout structures
	Renderer::IOEVertexFormatContainer *pManager = Renderer::IOEVertexFormatContainer::GetSingletonPtr();
	Renderer::IOEVertexFormat *pVertexFormat	 = pManager->CreateVertexFormat("DebugDrawOctree").get();
	pVertexFormat->AddSemanticFormat("POSITION", 0, Renderer::ETextureFormat::R32G32B32A32_FLOAT, 0,
									 offsetof(DebugOctreeVertex, vPos), Renderer::EVertexClassification::PerVertex, 0);
	pVertexFormat->AddSemanticFormat("TEXCOORD", 0, Renderer::ETextureFormat::R32G32B32_FLOAT, 0,
									 offsetof(DebugOctreeVertex, arrOffset[0]),
									 Renderer::EVertexClassification::PerVertex, 0);

	Renderer::IOEVertexFormat *pPerPixelLinkedListVertexFormat =
		pManager->CreateVertexFormat("DebugPerPixelLinkedListVertex").get();
	pPerPixelLinkedListVertexFormat->AddSemanticFormat("POSITION", 0, Renderer::ETextureFormat::R32G32B32A32_FLOAT, 0,
													   offsetof(DebugPerPixelLinkedListVertex, vPos),
													   Renderer::EVertexClassification::PerVertex, 0);
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Compile the generic diffuse material
	m_pDiffuseMaterial = std::make_shared<Renderer::IOEMaterial>(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"DefaultDiffuse.ioe.xml"));
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Compile the phong based bump mapping material
	m_pBumpMappingMaterial = std::make_shared<Renderer::IOEMaterial>();
#if (ENABLE_SHADOW_MAPPING == 1)
#if (DUAL_PARABALOID_SHADOW_MAPPING == 1)
	m_pBumpMappingMaterial->AddDefinition("SHADOW_MAPPING", "2");
#else
	m_pBumpMappingMaterial->AddDefinition("SHADOW_MAPPING", "1");
#endif
#endif
	m_pBumpMappingMaterial->LoadMaterial(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"DefaultBumpMapping.ioe.xml"));
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Compile the physical based material
	for (int nMaterialIdx = 0; nMaterialIdx < 3; ++nMaterialIdx)
	{
		m_pPhysicalMaterial[nMaterialIdx] = std::make_shared<Renderer::IOEMaterial>();
		if (nMaterialIdx != 2)
		{
#if (ENABLE_VOXEL_CONE_TRACING == 1)
			m_pPhysicalMaterial[nMaterialIdx]->AddDefinition("VOXEL_CONE_TRACING", "1");
#elif (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
			m_pPhysicalMaterial[nMaterialIdx]->AddDefinition("BIDIRECTIONAL_PATH_TRACING", "1");
#endif
		}

		if (nMaterialIdx == 1)
		{
			m_pPhysicalMaterial[nMaterialIdx]->AddDefinition("NO_DIRECT_LIGHT", "1");
		}

#if (ENABLE_SHADOW_MAPPING == 1)
#if (DUAL_PARABALOID_SHADOW_MAPPING == 1)
		m_pPhysicalMaterial[nMaterialIdx]->AddDefinition("SHADOW_MAPPING", "2");
#else
		m_pPhysicalMaterial[nMaterialIdx]->AddDefinition("SHADOW_MAPPING", "1");
#endif
#endif
		m_pPhysicalMaterial[nMaterialIdx]->LoadMaterial(
			Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"DefaultPhysical.ioe.xml"));
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
		m_pPhysicalMaterial[nMaterialIdx]->MapCBuffer("RarelyModified");
		m_pPhysicalMaterial[nMaterialIdx]->SetShaderVariableValue("RarelyModified", "ScreenWidth",
																  BidirectionalPathTracer::ms_fVPLResolution);
		m_pPhysicalMaterial[nMaterialIdx]->SetShaderVariableValue("RarelyModified", "ScreenHeight",
																  BidirectionalPathTracer::ms_fVPLResolution);
		m_pPhysicalMaterial[nMaterialIdx]->SetShaderVariableValue("RarelyModified", "NumSamples",
																  BidirectionalPathTracer::ms_nNumSamples);
		m_pPhysicalMaterial[nMaterialIdx]->UnmapCBuffer("RarelyModified");
#endif
	}
//
//////////////////////////////////////////////////////////////////////////

// Assign the initial material
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 0 && ENABLE_VOXEL_CONE_TRACING == 0)
	m_pActiveMaterial = m_pBumpMappingMaterial;
#else
	m_pActiveMaterial = m_pPhysicalMaterial[0];
#endif

	//////////////////////////////////////////////////////////////////////////
	// Compile all 7 copy buffer shaders, each one takes a single side of the
	// face
	// and the last uses a texture3 to do a 3D lookup
	for (int32_t nIdx(0); nIdx < 7; ++nIdx)
	{
		m_pCopyBufferMaterial[nIdx] = std::make_shared<Renderer::IOEMaterial>();
		if (nIdx > 0)
		{
			char buffer[2] = { 0 };
			_itoa_s<2>(nIdx - 1, buffer, 10);
			m_pCopyBufferMaterial[nIdx]->AddDefinition("TEXTURE_CUBE_DIRECTION", buffer);
		}
		m_pCopyBufferMaterial[nIdx]->LoadMaterial(
			Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Materials", L"CopyBuffer.ioe.xml"));
	}
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Load the scene and add to renderer
	std::wstring szModelPath;
#if 0
	szModelPath = Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Models", L"Farmhouse.fbx");
#else
	szModelPath = Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(), L"Models", L"Sponza.fbx");
#endif
	m_pSponza = IOE::Renderer::IOEModelManager::GetSingletonPtr()->LoadModel(szModelPath);
	m_pSponza->SetMaterial(IOE::Renderer::IOERPI::GetSingletonPtr(), m_pActiveMaterial);
	IOE::Renderer::IOEModelManager::GetSingletonPtr()->AddToRender(m_pSponza);

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Initialisation
	UpdateIntensity();
	SetLightPosition(m_vLightPos);
	SetLightColour(m_vLightColour);
	m_tOrthographicCamera.SetOrthographic();
	SetupFullscreenBuffers(Renderer::IOERPI::GetSingletonPtr());

#if (ENABLE_VOXEL_CONE_TRACING == 1)
	m_tVoxelConeTracer.Initialise();
#endif
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
	m_tBidirectionalPathTracer.Initialise();
#endif
#if (ENABLE_SHADOW_MAPPING == 1)
	m_tShadowMap.Initialise();
#endif

	m_tDefaultRasterizerState.Setup(Renderer::IOERPI::GetSingletonPtr());
	m_tNoClipRasterizerState.Setup(Renderer::IOERPI::GetSingletonPtr());

	m_pDefaultSampler->Setup(Renderer::IOERPI::GetSingletonPtr());
	m_pClampSampler->Setup(Renderer::IOERPI::GetSingletonPtr());
	//
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// HUD Stuff
	m_pFont = Renderer::IOEFont::CreateFontFromName("Arial", 16);
	m_pFont->Setup(Renderer::IOERPI::GetSingletonPtr());

	std::uint32_t uX, uY;
	GetWindowSize(uX, uY);

	m_tHUD.SetScreenSize(static_cast<float>(uX), static_cast<float>(uY));
	m_tHUD.GenerateText(m_pFont);
	m_tHUD.m_tFrameDelta.SetDataSource(&m_fLastRender);
	m_tHUD.m_tIntensity.SetDataSource(&m_fIntensity);
	m_tHUD.m_tRoughness.SetDataSource(&m_fRoughness);
	m_tHUD.m_tSpecular.SetDataSource(&m_fLightSpecular);
	m_tHUD.m_tDebugMode.SetDataSource(&m_szDebugMode);
	m_tHUD.m_tIndirectDirectLight.SetDataSource(&m_szLightingMode);
	//
	//////////////////////////////////////////////////////////////////////////

	UnserialiseData();
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::OnResize(std::uint32_t uWidth, std::uint32_t uHeight)
{
	IOEApplication::OnResize(uWidth, uHeight);

	m_tHUD.SetScreenSize(static_cast<float>(uWidth), static_cast<float>(uHeight));
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::HandleInput(IOE::Core::IOETimeDelta tTimeDelta)
{
	static const float fIntensityScale = 50.0f;
	static const float fRoughnessScale = 0.1f;
	static const float fSpecularScale  = 0.05f;

	Input::IOEInputKeyboard *pKeyboard(
		IOE::Input::IOEInputManager::GetSingletonPtr()->FindDevice<IOE::Input::IOEInputKeyboard>());
	Input::IOEInputMouse *pMouse(
		IOE::Input::IOEInputManager::GetSingletonPtr()->FindDevice<IOE::Input::IOEInputMouse>());

	if (pKeyboard)
	{
		//////////////////////////////////////////////////////////////////////////
		// Debug modes! F1 = next, F2 = previous
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::F1))
		{
			m_eDebugDrawMode = GetNextDebugDrawMode(m_eDebugDrawMode);
			m_szDebugMode	= GetDebugDrawModeName(m_eDebugDrawMode);
			if (m_eDebugDrawMode == EDebugDrawMode::DrawCameraOrientation)
			{
				m_pActiveMaterial = m_pDiffuseMaterial;
			}
			else
			{
				m_pActiveMaterial = m_pPhysicalMaterial[0];
			}

			IOE::Core::g_pEngine->GetApplication()->Logf("Changed Debug Draw Mode: %s\n", m_szDebugMode);
		}
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::F2))
		{
			m_eDebugDrawMode = GetPreviousDebugDrawMode(m_eDebugDrawMode);
			m_szDebugMode	= GetDebugDrawModeName(m_eDebugDrawMode);

			IOE::Core::g_pEngine->GetApplication()->Logf("Changed Debug Draw Mode: %s\n", m_szDebugMode);
		}
		//
		//////////////////////////////////////////////////////////////////////////

		bool bChanged(false);

		//////////////////////////////////////////////////////////////////////////
		// F3 = Bump mapping material, F4 = PBR material
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::F3))
		{
			if (m_pActiveMaterial != m_pBumpMappingMaterial)
			{
				m_pActiveMaterial = m_pBumpMappingMaterial;
				bChanged		  = true;
			}
		}
		else if (pKeyboard->JustReleased(IOE::Input::EInputKey::F4))
		{
			if (m_pActiveMaterial != m_pPhysicalMaterial[m_nLightingMode])
			{
				m_pActiveMaterial = m_pPhysicalMaterial[m_nLightingMode];
				bChanged		  = true;
			}
		}

		// Toggle lighting mode on I
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::I))
		{
			m_nLightingMode = (m_nLightingMode + 1) % 3;
			switch (m_nLightingMode)
			{
			case 0:
				m_szLightingMode = "Direct And Indirect";
				break;
			case 1:
				m_szLightingMode = "Only Indirect";
				break;
			case 2:
				m_szLightingMode = "Only Direct";
				break;
			}
			if (m_pActiveMaterial == m_pPhysicalMaterial[0] || m_pActiveMaterial == m_pPhysicalMaterial[1] ||
				m_pActiveMaterial == m_pPhysicalMaterial[2])
			{
				m_pActiveMaterial = m_pPhysicalMaterial[m_nLightingMode];
				bChanged		  = true;
			}
		}

		if (pKeyboard->JustReleased(IOE::Input::EInputKey::O))
		{
			m_nCurrentSerialisedDataIdx = m_nCurrentSerialisedDataIdx - 1;
			if (m_nCurrentSerialisedDataIdx < 0)
			{
				m_nCurrentSerialisedDataIdx += (int)m_tSerialisedData.uNumElements;
			}
			if (m_nCurrentSerialisedDataIdx >= 0)
			{
				m_tSerialisedData.SetSettings(m_nCurrentSerialisedDataIdx);
			}
		}
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::P))
		{
			if (m_tSerialisedData.uNumElements > 0)
			{
				m_nCurrentSerialisedDataIdx = ((m_nCurrentSerialisedDataIdx + 1) % m_tSerialisedData.uNumElements);
				m_tSerialisedData.SetSettings(m_nCurrentSerialisedDataIdx);
			}
		}

		if (bChanged)
		{
			m_pSponza->SetMaterial(IOE::Renderer::IOERPI::GetSingletonPtr(), m_pActiveMaterial);
			SetLightPosition(m_vLightPos);
		}
		//
		//////////////////////////////////////////////////////////////////////////

		bChanged = false;

		//////////////////////////////////////////////////////////////////////////
		// Intensity -(F5) and +(F6), Roughness -(F7) and +(F8)
		if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::F5))
		{
			m_fRoughness -= fRoughnessScale * tTimeDelta.fDelta;
			m_fLightSpecular += fSpecularScale * tTimeDelta.fDelta;
			bChanged = true;
		}
		else if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::F6))
		{
			m_fRoughness += fRoughnessScale * tTimeDelta.fDelta;
			m_fLightSpecular -= fSpecularScale * tTimeDelta.fDelta;
			bChanged = true;
		}

		if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::F7))
		{
			m_fIntensity -= fIntensityScale * tTimeDelta.fDelta;
			bChanged = true;
		}
		else if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::F8))
		{
			m_fIntensity += fIntensityScale * tTimeDelta.fDelta;
			bChanged = true;
		}
		m_fIntensity	 = IOE::Maths::Clamp(m_fIntensity, 1.0f, 1000.0f);
		m_fRoughness	 = IOE::Maths::Clamp(m_fRoughness, 0.1f, 1.0f);
		m_fLightSpecular = IOE::Maths::Clamp(m_fLightSpecular, 0.04f, 0.5f);

		if (bChanged)
		{
			UpdateIntensity();
		}
		//
		//////////////////////////////////////////////////////////////////////////

		// Toggle HUD visiblity on SPACE
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::Space))
		{
			Renderer::IOETextManager::GetSingletonPtr()->SetVisible(
				!Renderer::IOETextManager::GetSingletonPtr()->GetVisible());
		}

		//////////////////////////////////////////////////////////////////////////
		// Camera controls: WASD = movement, Arrow Keys = Rotation
		Maths::IOECamera &rCamera(Renderer::IOERPI::GetSingletonPtr()->GetDefaultCamera());
		Maths::IOETransform rTransform(m_eCameraMode == ECameraMode::Camera ? m_vCameraPosition : m_vLightPos,
									   m_qCameraRotation);

		if (!pKeyboard->IsKeyDown(Input::EInputKey::LCtrl))
		{
			bool bForward(pKeyboard->IsKeyDown(IOE::Input::EInputKey::W)); // W
			bool bBack(pKeyboard->IsKeyDown(IOE::Input::EInputKey::S));	// S
			bool bLeft(pKeyboard->IsKeyDown(IOE::Input::EInputKey::A));	// A
			bool bRight(pKeyboard->IsKeyDown(IOE::Input::EInputKey::D));   // D

			bool bRotLeft(pKeyboard->IsKeyDown(IOE::Input::EInputKey::Left));   // Left arrow
			bool bRotRight(pKeyboard->IsKeyDown(IOE::Input::EInputKey::Right)); // Right arrow
			bool bPitchUp(pKeyboard->IsKeyDown(IOE::Input::EInputKey::Up));		// Up Aarrow
			bool bPitchDown(pKeyboard->IsKeyDown(IOE::Input::EInputKey::Down)); // Down Arrow

			if (bForward || bBack || bLeft || bRight || bRotLeft || bRotRight || bPitchDown || bPitchUp)
			{
				m_bCameraDirty = true;
			}

			const float fSpeed	= 5.0f * Core::g_pEngine->GetFrameDelta().fDelta;
			const float fRotSpeed = 60.0f * Core::g_pEngine->GetFrameDelta().fDelta;

			if (bForward)
				rTransform.SetTranslation(rTransform.GetTranslation() +
										  rTransform.GetRotationQuat().Rotate(Maths::g_vForwardVector) * fSpeed);
			if (bBack)
				rTransform.SetTranslation(rTransform.GetTranslation() -
										  rTransform.GetRotationQuat().Rotate(Maths::g_vForwardVector) * fSpeed);
			if (bLeft)
				rTransform.SetTranslation(rTransform.GetTranslation() -
										  rTransform.GetRotationQuat().Rotate(Maths::g_vSideVector) * fSpeed);
			if (bRight)
				rTransform.SetTranslation(rTransform.GetTranslation() +
										  rTransform.GetRotationQuat().Rotate(Maths::g_vSideVector) * fSpeed);

			float fPitch = rTransform.GetRotationQuat().GetPitch();
			float fYaw   = rTransform.GetRotationQuat().GetYaw();

			if (bRotLeft)
				fYaw -= fRotSpeed;
			if (bRotRight)
				fYaw += fRotSpeed;
			if (bPitchUp)
				fPitch -= fRotSpeed;
			if (bPitchDown)
				fPitch += fRotSpeed;

			rTransform.SetRotationQuat(Maths::IOEQuaternion(fPitch, fYaw, 0.0f));

			rCamera.SetUpVector(Maths::g_vUpVector);
			rCamera.SetLookAt(rTransform.GetTranslation() +
							  rTransform.GetRotationQuat().Rotate(Maths::g_vForwardVector));
			rCamera.GetTransform() = rTransform;
			rCamera.UpdateViewMatrices();
		}

		if (m_bCameraDirty)
		{
			m_tCameraVisibilityCheck.UpdateVisibility(rCamera);
			m_bCameraDirty = false;

			if (m_eCameraMode == ECameraMode::Camera)
			{
				m_vCameraPosition = rTransform.GetTranslation();
			}
			else
			{
				SetLightPosition(rTransform.GetTranslation());
			}
			m_qCameraRotation = rTransform.GetRotationQuat();
		}
		//
		//////////////////////////////////////////////////////////////////////////
	}

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
	IOE::Renderer::IOERPI::GetSingletonPtr()->AddDebugBox(
		m_tBidirectionalPathTracer.GetRayBundleCamera(0).GetTransform().GetTranslation(), ColourList::Aquamarine);
#endif // (ENABLE_BIDIRECTION_PATH_TRACER)

#if !defined(IOE_FINAL)
	HandleInput_Debug(tTimeDelta);
#endif
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::HandleInput_Debug(IOE::Core::IOETimeDelta tTimeDelta)
{
	Input::IOEInputKeyboard *pKeyboard(
		IOE::Input::IOEInputManager::GetSingletonPtr()->FindDevice<IOE::Input::IOEInputKeyboard>());
	Input::IOEInputMouse *pMouse(
		IOE::Input::IOEInputManager::GetSingletonPtr()->FindDevice<IOE::Input::IOEInputMouse>());

	if (pKeyboard)
	{
		if (pKeyboard->JustReleased(Input::EInputKey::S) && pKeyboard->IsKeyDown(Input::EInputKey::LCtrl))
		{
			SerialiseData();
		}
		if (pKeyboard->JustReleased(Input::EInputKey::A) && pKeyboard->IsKeyDown(Input::EInputKey::LCtrl))
		{
			m_tSerialisedData.AddNewData();
			m_nCurrentSerialisedDataIdx = (int)m_tSerialisedData.uNumElements - 1;
		}
		if (pKeyboard->JustReleased(Input::EInputKey::D) && pKeyboard->IsKeyDown(Input::EInputKey::LCtrl))
		{
			if (m_nCurrentSerialisedDataIdx >= 0)
			{
				m_tSerialisedData.RemoveIndex(m_nCurrentSerialisedDataIdx);
			}
			m_nCurrentSerialisedDataIdx = m_nCurrentSerialisedDataIdx - 1;
			if (m_nCurrentSerialisedDataIdx >= 0)
			{
				m_tSerialisedData.SetSettings(m_nCurrentSerialisedDataIdx);
			}
		}

		auto pRPI(Renderer::IOERPI::GetSingletonPtr());

		Maths::IOECamera &rCamera(pRPI->GetDefaultCamera());

		Maths::IOETransform rTransform(m_eCameraMode == ECameraMode::Camera ? m_vCameraPosition : m_vLightPos,
									   m_qCameraRotation);

		if (pKeyboard->JustReleased(IOE::Input::EInputKey::Tilde))
		{
			if (pMouse->IsLocked())
			{
				pMouse->UnlockCursor();
			}
			else
			{
				pMouse->LockCursor();
			}
		}

		static float fTimer(0.0f);
		static Maths::IOERay tRay(Maths::g_vIdentity, Maths::g_vIdentity);

		if (fTimer > 0.0f)
		{
			fTimer -= IOE::Core::g_pEngine->GetFrameDelta().fDelta;
			if (fTimer < 0.0f)
				fTimer = 0.0f;

			for (int32_t nIdx(0); nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels(); ++nIdx)
			{
				Renderer::IOEModel *pModel(Renderer::IOEModelManager::GetSingletonPtr()->GetModel(nIdx));

				pRPI->AddDebugLine(tRay.m_vOrigin, tRay.m_vOrigin + tRay.m_vDirAndLength, Maths::ColourList::Purple,
								   Maths::ColourList::White);

				for (int32_t nSubset(0); nSubset < pModel->GetNumSubsets(); ++nSubset)
				{
					const Renderer::MeshIndex &rIndex(pModel->GetSubset(nSubset));

					Maths::IOEVector vColour(Maths::ColourList::Red);

					if (rIndex.m_tAABB.Intersect(tRay))
					{
						vColour = Maths::ColourList::Green;
					}

					pRPI->AddDebugBox((rIndex.m_tAABB.m_vBounds[0] + rIndex.m_tAABB.m_vBounds[1]) * 0.5f,
									  (rIndex.m_tAABB.m_vBounds[1] - rIndex.m_tAABB.m_vBounds[0]) * 0.5f);
				}
			}
		}

		if (pKeyboard->JustReleased(IOE::Input::EInputKey::Return))
		{
			m_eCameraMode  = m_eCameraMode == ECameraMode::Camera ? ECameraMode::Light : ECameraMode::Camera;
			m_bCameraDirty = true;
		}

		if (pMouse->JustPressed(Input::EInputMouseButton::MouseButton1) && !pMouse->IsLocked())
		{
			fTimer = 10.0f;

			int32_t nX, nY;
			pMouse->GetAbsolutePosition(nX, nY);
			IOEVector vDir(rCamera.ScreenSpaceDirToWorldSpaceDir(nX, nY));
			const IOEVector &vTranslation(rCamera.GetTransform().GetTranslation());
			tRay = Maths::IOERay(vTranslation, vDir * 100.0f);
		}

#if (ENABLE_VOXEL_CONE_TRACING == 1)
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::Digit6))
		{
			m_tVoxelConeTracer.ToggleDebugDraw();
		}

		if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::Digit0))
		{
			m_tVoxelConeTracer.ModifyLOD(Core::g_pEngine->GetFrameDelta().fDelta * 4.0f);
		}
		else if (pKeyboard->IsKeyDown(IOE::Input::EInputKey::Digit9))
		{
			m_tVoxelConeTracer.ModifyLOD(Core::g_pEngine->GetFrameDelta().fDelta * -4.0f);
		}
		if (pKeyboard->JustReleased(IOE::Input::EInputKey::Digit8))
		{
			m_tVoxelConeTracer.ModifyAxis(1);
		}
		else if (pKeyboard->JustReleased(IOE::Input::EInputKey::Digit7))
		{
			m_tVoxelConeTracer.ModifyAxis(-1);
		}
#endif
	}
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::OnUpdate(IOE::Core::IOETimeDelta tTimeDelta)
{
	IOEApplication::OnUpdate(tTimeDelta);

	m_tHUD.Update();

	HandleInput(tTimeDelta);
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::UpdateIntensity()
{
	m_pActiveMaterial->MapCBuffer("LightSettings");
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "Intensity", m_fIntensity);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "Roughness", m_fRoughness);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "LightPos", m_vLightPos);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "LightColour", m_vLightColour);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "LightSpecular", m_fLightSpecular);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "LightRadius", m_fLightRadius);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "Near", 0.5f);
	m_pActiveMaterial->SetShaderVariableValue("LightSettings", "Far", 1000.0f);
	m_pActiveMaterial->UnmapCBuffer("LightSettings");

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
	m_tBidirectionalPathTracer.SetLightSettings(m_vLightPos, m_fIntensity);
#endif
#if (ENABLE_VOXEL_CONE_TRACING == 1)
	m_tVoxelConeTracer.UpdateLightSettings();
#endif
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::OnRender(IOE::Renderer::IOERPI *pRPI)
{
	START_PROFILE_MARKER("Scene", Maths::ColourList::Red);
	{

		IOEApplication::OnRender(pRPI);

// Render shadow mapping first
#if (ENABLE_SHADOW_MAPPING == 1)
		START_PROFILE_MARKER("Shadow Mapping", Maths::ColourList::Blue);
		m_tShadowMap.PreRender(pRPI);
		m_tShadowMap.Render(pRPI);
		m_tShadowMap.PostRender(pRPI);
		END_PROFILE_MARKER();
#endif

// Render voxel cone tree octree if needed and perform the lodding step
#if (ENABLE_VOXEL_CONE_TRACING == 1)
		START_PROFILE_MARKER("Voxel Cone Tracing", Maths::ColourList::Pink);
		m_tVoxelConeTracer.PreRender(pPtr);
#if (ENABLE_SHADOW_MAPPING == 1)
		m_tVoxelConeTracer.SetShadowMap(m_tShadowMap.GetShadowMap());
#endif
		m_tVoxelConeTracer.Render(pPtr);
		m_tVoxelConeTracer.PostRender(pPtr);
		END_PROFILE_MARKER();
#endif

// Render VPL shadow maps and global ray bundles if needed
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
		START_PROFILE_MARKER("Bidirectional Raytracing", Maths::ColourList::Pink);
		m_tBidirectionalPathTracer.PreRender(pRPI);
		m_tBidirectionalPathTracer.Render(pRPI);
		m_tBidirectionalPathTracer.PostRender(pRPI);
		END_PROFILE_MARKER();
#endif

		START_PROFILE_MARKER("Final Gather", Maths::ColourList::Orange);

// Perform the final gather
#if (ENABLE_SHADOW_MAPPING == 1)
		m_pActiveMaterial->SetTextureVariable(pRPI, "ShadowMap", m_tShadowMap.GetShadowMap().get());
		m_pActiveMaterial->SetSamplerVariable("g_ShadowSampler", m_pClampSampler.get());
#if (DUAL_PARABALOID_SHADOW_MAPPING == 1)
		m_pActiveMaterial->MapCBuffer("ParabaloidShadowMapping");
		m_pActiveMaterial->SetShaderVariableValue("ParabaloidShadowMapping", "ParabaloidView",
												  m_tShadowMap.GetCamera(0).GetViewMatrix());
		m_pActiveMaterial->UnmapCBuffer("ParabaloidShadowMapping");
#endif
#endif

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
		m_pActiveMaterial->SetTextureVariable(pRPI, "InOutFragmentAndLink",
											  m_tBidirectionalPathTracer.GetPixelLinkedListBuffer());
		m_pActiveMaterial->SetTextureVariable(pRPI, "InFragmentListHead", m_tBidirectionalPathTracer.GetHeadBuffer(),
											  -1);
		m_pActiveMaterial->SetTextureVariable(pRPI, "InRayBundleCameraLocations",
											  m_tBidirectionalPathTracer.GetRayBundleCameraLocationBuffer());
		m_pActiveMaterial->SetTextureVariable(pRPI, "InVPLSampleData",
											  m_tBidirectionalPathTracer.GetVPLSampleDataBuffer());
		m_pActiveMaterial->SetTextureVariable(pRPI, "InVPLShadowMaps", m_tBidirectionalPathTracer.GetVPLShadowMaps());
#endif

		if ((m_eDebugDrawMode == EDebugDrawMode::None)
#if (ENABLE_VOXEL_CONE_TRACING == 1)
			&& !m_tVoxelConeTracer.IsDebugDrawEnabled()
#endif
		)
		{
			IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(pRPI, m_tCameraVisibilityCheck);
		}

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
		m_pActiveMaterial->SetTextureVariable(pRPI, "InVPLShadowMaps", nullptr);
		m_pActiveMaterial->SetTextureVariable(pRPI, "InVPLSampleData", nullptr);
		m_pActiveMaterial->SetTextureVariable(pRPI, "InRayBundleCameraLocations", nullptr);
		m_pActiveMaterial->SetTextureVariable(pRPI, "InFragmentListHead", nullptr);
		m_pActiveMaterial->SetTextureVariable(pRPI, "InOutFragmentAndLink", nullptr);
#endif

#if (ENABLE_SHADOW_MAPPING == 1)
		m_pActiveMaterial->SetTextureVariable(pRPI, "ShadowMap", nullptr);
#endif

		END_PROFILE_MARKER();

#if (ENABLE_VOXEL_CONE_TRACING == 1)
		m_tVoxelConeTracer.FinishRender(pPtr);
#endif

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
		m_tBidirectionalPathTracer.FinishRender(pRPI);
#endif
	}
	END_PROFILE_MARKER();

	START_PROFILE_MARKER("Debug Drawing", Maths::ColourList::Pink);
	DebugDraw();
	END_PROFILE_MARKER();

	m_fLastRender = (0.75f * m_fLastRender) + (0.25f * Core::g_pEngine->GetFrameDelta().fInvDelta);
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::SetupFullscreenBuffers(Renderer::IOERPI *pRPI)
{
	m_tFullscreenBuffer.SetEmplace(0, IOEVector(1.0f, 1.0f, 0.0f), 0.0f, 0.0f, 0.0f);
	m_tFullscreenBuffer.SetEmplace(1, IOEVector(1.0f, -1.0f, 0.0f), 0.0f, 1.0f, 0.0f);
	m_tFullscreenBuffer.SetEmplace(2, IOEVector(-1.0f, 1.0f, 0.0f), 1.0f, 0.0f, 0.0f);

	m_tFullscreenBuffer.SetEmplace(3, IOEVector(-1.0f, 1.0f, 0.0f), 1.0f, 0.0f, 0.0f);
	m_tFullscreenBuffer.SetEmplace(4, IOEVector(1.0f, -1.0f, 0.0f), 0.0f, 1.0f, 0.0f);
	m_tFullscreenBuffer.SetEmplace(5, IOEVector(-1.0f, -1.0f, 0.0f), 1.0f, 1.0f, 0.0f);

	m_tFullscreenBuffer.Setup(pRPI);
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::CopyFullscreenTarget(IOE::Renderer::IOERPI *pRPI, Renderer::IOETexture *pSrcTexture,
											IOE::Renderer::IOEMaterial *pSrcMaterial /*=nullptr*/,
											Renderer::ECubeFace eFace, float fLightModifier /*=1.0f*/)
{
	pRPI->SetDepthTarget(pRPI->GetDefaultDepthStencilbuffer(), false);
	pRPI->SetRenderTarget(pRPI->GetDefaultColourBuffer());

	pRPI->SetCamera(m_tOrthographicCamera);

	pRPI->Clear(IOEVector(0.0f, 0.0f, 1.0f, 1.0f));

	auto pMaterial(pSrcMaterial == nullptr ? m_pCopyBufferMaterial[0].get() : pSrcMaterial);
	if (pSrcMaterial == nullptr && pSrcTexture->GetTextureType() == Renderer::ETextureType::TextureCube)
	{
		pMaterial = m_pCopyBufferMaterial[((int32_t)eFace) + 1].get();
	}
	pMaterial->Set(IOE::Renderer::IOERPI::GetSingletonPtr());

	pMaterial->SetTextureVariable(pRPI, "DiffuseTexture", pSrcTexture);

	pMaterial->MapCBuffer("Globals");
	pMaterial->SetShaderVariableValue("Globals", "WorldViewProj", m_tOrthographicCamera.GetProjectionMatrix());
	pMaterial->SetShaderVariableValue("Globals", "LightModifier", fLightModifier);
	pMaterial->UnmapCBuffer("Globals");

	pRPI->DrawBuffer(&m_tFullscreenBuffer, Renderer::EPrimitiveTopology::TriangleList);

	pMaterial->SetTextureVariable(pRPI, "DiffuseTexture", nullptr);

	pRPI->SetCamera(pRPI->GetDefaultCamera());
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::SetLightPosition(const IOE::Maths::IOEVector &vLightPosition)
{
	m_vLightPos = vLightPosition;
	UpdateIntensity();

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
	m_tBidirectionalPathTracer.SetLightSettings(m_vLightPos, m_fIntensity);
#endif
#if (ENABLE_SHADOW_MAPPING == 1)
	m_tShadowMap.SetLightPosition(vLightPosition);
#endif
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::SetLightColour(const IOE::Maths::IOEVector &vLightColour)
{
	m_vLightColour = vLightColour;
	UpdateIntensity();
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::SerialiseData()
{
	std::ofstream tStream("_data.bin", std::ios::out | std::ios::binary);

	tStream.write((const char *)(&m_tSerialisedData.uNumElements), sizeof(m_tSerialisedData.uNumElements));
	for (SerialisedData::_InternalData &rData : m_tSerialisedData.m_arrData)
	{
		tStream.write((const char *)(&rData.LightPosition), sizeof(float) * 3);
		tStream.write((const char *)(&rData.LightColour), sizeof(float) * 4);
		tStream.write((const char *)(&rData.LightSpecular), sizeof(float) * 1);
		tStream.write((const char *)(&rData.LightIntensity), sizeof(float) * 1);
		tStream.write((const char *)(&rData.LightRoughness), sizeof(float) * 1);
		tStream.write((const char *)(&rData.LightRadius), sizeof(float) * 1);

		tStream.write((const char *)(&rData.CameraProperties), sizeof(Maths::IOECamera::CCameraProperties));
		tStream.write((const char *)(&rData.CameraTransform), sizeof(Maths::IOEMatrix));
		tStream.write((const char *)(&rData.CameraLookAt), sizeof(float) * 3);
	}

	/*XMVECTOR tVector(m_vLightPos.GetRaw());
	tStream.write((const char*)(&tVector), sizeof(XMVECTOR));

	tVector = m_vLightColour.GetRaw();
	tStream.write((const char*)(&tVector), sizeof(XMVECTOR));

	tStream.write((const char*)(&m_fLightSpecular), sizeof(float));
	tStream.write((const char*)(&m_fIntensity), sizeof(float));
	tStream.write((const char*)(&m_fRoughness), sizeof(float));

	IOE::Maths::IOECamera
	&rCamera(IOE::Renderer::IOERPI::GetSingletonPtr()->GetDefaultCamera());
	const IOE::Maths::IOECamera::CCameraProperties
	&rCameraProperties(rCamera.GetCameraProperties());
	const XMMATRIX &tTransform(rCamera.GetTransform().ToMatrix().GetRaw());
	XMVECTOR vLookAt(rCamera.GetLookAt().GetRaw());


	tStream.write((const char*)(&rCameraProperties),
			sizeof(IOE::Maths::IOECamera::CCameraProperties));
	tStream.write((const char*)(&tTransform),
			sizeof(XMMATRIX));
	tStream.write((const char*)(&vLookAt),
			sizeof(XMVECTOR));*/

	tStream.close();
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::UnserialiseData()
{
	std::ifstream tStream("_data.bin", std::ios::in | std::ios::binary);
	if (tStream.is_open())
	{
		char buffer[64];

		tStream.read(buffer, sizeof(std::uint32_t));
		m_tSerialisedData.uNumElements = *((std::uint32_t *)buffer);

		m_tSerialisedData.m_arrData.resize(m_tSerialisedData.uNumElements);

		for (std::uint32_t uIdx(0); uIdx < m_tSerialisedData.uNumElements; ++uIdx)
		{
			SerialisedData::_InternalData &rData(m_tSerialisedData.m_arrData[uIdx]);

			tStream.read(buffer, sizeof(float) * 3);
			memcpy(&rData.LightPosition[0], buffer, sizeof(float) * 3);

			tStream.read(buffer, sizeof(float) * 4);
			memcpy(&rData.LightColour[0], buffer, sizeof(float) * 4);

			tStream.read(buffer, sizeof(float));
			rData.LightSpecular = *((float *)buffer);

			tStream.read(buffer, sizeof(float));
			rData.LightIntensity = *((float *)buffer);

			tStream.read(buffer, sizeof(float));
			rData.LightRoughness = *((float *)buffer);

			tStream.read(buffer, sizeof(float));
			rData.LightRadius = *((float *)buffer);

			tStream.read(buffer, sizeof(Maths::IOECamera::CCameraProperties));
			rData.CameraProperties = *((Maths::IOECamera::CCameraProperties *)buffer);

			tStream.read(buffer, sizeof(Maths::IOEMatrix));
			rData.CameraTransform = *((Maths::IOEMatrix *)buffer);

			tStream.read(buffer, sizeof(float) * 3);
			memcpy(&rData.CameraLookAt[0], buffer, sizeof(float) * 3);
		}

		/*tStream.read(buffer, sizeof(XMVECTOR));
		m_vLightPos = *((XMVECTOR*)buffer);

		tStream.read(buffer, sizeof(XMVECTOR));
		m_vLightColour = *((XMVECTOR*)buffer);

		tStream.read(buffer, sizeof(float));
		m_fLightSpecular = *((float*)buffer);

		tStream.read(buffer, sizeof(float));
		m_fIntensity = *((float*)buffer);

		tStream.read(buffer, sizeof(float));
		m_fRoughness = *((float*)buffer);

		tStream.read(buffer, sizeof(IOE::Maths::IOECamera::CCameraProperties));
		IOE::Maths::IOECamera
		&rCamera(IOE::Renderer::IOERPI::GetSingletonPtr()->GetDefaultCamera());
		rCamera.SetCameraProperties(*((IOE::Maths::IOECamera::CCameraProperties*)buffer));

		tStream.read(buffer, sizeof(XMMATRIX));
		rCamera.GetTransform().FromMatrix(*((XMMATRIX*)buffer));

		tStream.read(buffer, sizeof(XMVECTOR));
		rCamera.SetLookAt(*((XMVECTOR*)buffer));

		rCamera.UpdateViewMatrices();

		m_bCameraDirty = true;
		m_vCameraPosition = rCamera.GetTransform().GetTranslation();
		m_qCameraRotation = rCamera.GetTransform().GetRotationQuat();

		m_tCameraVisibilityCheck.UpdateVisibility(rCamera);*/

		tStream.close();

		m_tSerialisedData.SetSettings(0);
		m_nCurrentSerialisedDataIdx = 0;
	}

	/*SetLightPosition(m_vLightPos);
	SetLightColour(m_vLightColour);
	UpdateIntensity();*/
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::DebugDraw()
{
	auto pPtr(Renderer::IOERPI::GetSingletonPtr());
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)

	if (m_eDebugDrawMode == EDebugDrawMode::DrawCameraOrientation)
	{
		static float s_fTest(0.0f);
		s_fTest += Core::g_pEngine->GetFrameDelta().fDelta;

		static int s_nIdx(0);

		if (s_fTest > 2.0f)
		{
			s_fTest -= 2.0f;
			if (++s_nIdx >= BidirectionalPathTracer::ms_nNumSamples)
			{
				s_nIdx -= BidirectionalPathTracer::ms_nNumSamples;
			}
		}

		IOE::Maths::IOECamera &rCamera(m_tBidirectionalPathTracer.GetRayBundleCamera(s_nIdx));

		pPtr->SetCamera(rCamera);

		Maths::IOEAABB tAABBTransformed = Maths::IOEAABB(m_pSponza->GetAABB());
		Maths::IOEVector vMin(tAABBTransformed.m_vBounds[0]);
		Maths::IOEVector vMax(tAABBTransformed.m_vBounds[1]);
		float fMaxBounds((vMax - vMin).MaxElement());

		pPtr->ResetViewports();
		pPtr->AddViewport(0.0f, 0.0f, fMaxBounds, fMaxBounds);
		pPtr->SetupViewports();

		pPtr->AddDebugBox((tAABBTransformed.m_vBounds[0] + tAABBTransformed.m_vBounds[1]) * 0.5f,
						  (tAABBTransformed.m_vBounds[1] - tAABBTransformed.m_vBounds[0]) * 0.5f);

		IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(pPtr, true);
	}
#endif
	pPtr->AddDebugBox(m_vLightPos, Maths::g_vOneVector * 0.1f);
}

//////////////////////////////////////////////////////////////////////////

void CTestApplication::SetCameraSettings(IOE::Maths::IOECamera &rCamera)
{
	rCamera.UpdateViewMatrices();

	m_vCameraPosition = rCamera.GetTransform().GetTranslation();
	m_qCameraRotation = rCamera.GetTransform().GetRotationQuat();

	m_bCameraDirty = true;

	m_tCameraVisibilityCheck.UpdateVisibility(rCamera);
}

//////////////////////////////////////////////////////////////////////////

void SerialisedData::AddNewData()
{
	_InternalData tInternalData;

	CTestApplication *pApp(CTestApplication::GetSingletonPtr());
	Renderer::IOERPI *pRPI(Renderer::IOERPI::GetSingletonPtr());

	// Cache current light settings
	pApp->GetLightPosition().Get(tInternalData.LightPosition[0], tInternalData.LightPosition[1],
								 tInternalData.LightPosition[2]);
	pApp->GetLightColour().Get(tInternalData.LightColour[0], tInternalData.LightColour[1],
							   tInternalData.LightColour[2], tInternalData.LightColour[3]);
	tInternalData.LightIntensity = pApp->GetLightIntensity();
	tInternalData.LightSpecular  = pApp->GetSpecular();
	tInternalData.LightRoughness = pApp->GetRoughness();
	tInternalData.LightRadius	= pApp->GetLightRadius();

	// Cache current camera settings
	Maths::IOECamera &rDefaultCamera(pRPI->GetDefaultCamera());
	tInternalData.CameraProperties = rDefaultCamera.GetCameraProperties();
	tInternalData.CameraTransform  = rDefaultCamera.GetTransform().ToMatrix();
	rDefaultCamera.GetLookAt().Get(tInternalData.CameraLookAt[0], tInternalData.CameraLookAt[1],
								   tInternalData.CameraLookAt[2]);

	// Add on the new internal data
	m_arrData.push_back(tInternalData);

	++uNumElements;

	pApp->SerialiseData();
}

//////////////////////////////////////////////////////////////////////////

void SerialisedData::SetSettings(std::int32_t nIdx)
{
	_InternalData &rInternalData = m_arrData.at(nIdx);

	CTestApplication *pApp(CTestApplication::GetSingletonPtr());
	Renderer::IOERPI *pRPI(Renderer::IOERPI::GetSingletonPtr());

	// Set Light Settings
	pApp->SetLightPosition(Maths::IOEVector(rInternalData.LightPosition[0], rInternalData.LightPosition[1],
											rInternalData.LightPosition[2]));
	pApp->SetLightColour(Maths::IOEVector(rInternalData.LightColour[0], rInternalData.LightColour[1],
										  rInternalData.LightColour[2], rInternalData.LightColour[3]));
	pApp->SetSpecular(rInternalData.LightSpecular);
	pApp->SetLightIntensity(rInternalData.LightIntensity);
	pApp->SetRoughness(rInternalData.LightRoughness);
	pApp->SetLightRadius(rInternalData.LightRadius);

	// Set camera settings
	Maths::IOECamera &rDefaultCamera(pRPI->GetDefaultCamera());
	rDefaultCamera.SetCameraProperties(rInternalData.CameraProperties);
	rDefaultCamera.GetTransform().FromMatrix(rInternalData.CameraTransform);
	rDefaultCamera.SetLookAt(
		Maths::IOEVector(rInternalData.CameraLookAt[0], rInternalData.CameraLookAt[1], rInternalData.CameraLookAt[2]));

	pApp->SetCameraSettings(rDefaultCamera);
}

//////////////////////////////////////////////////////////////////////////

void SerialisedData::RemoveIndex(std::int32_t nIdx)
{
	m_arrData.erase(std::begin(m_arrData) + nIdx);

	CTestApplication::GetSingletonPtr()->SerialiseData();
}

//////////////////////////////////////////////////////////////////////////
