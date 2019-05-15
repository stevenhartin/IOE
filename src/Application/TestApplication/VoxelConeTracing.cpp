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

#include "VoxelConeTracing.h"
#include "Main.h"

//////////////////////////////////////////////////////////////////////////

using namespace IOE;
using namespace IOE::Maths;
using Core::IOEManager;
using Core::IOEManagerContainer;
using Core::g_arrManagers;

//////////////////////////////////////////////////////////////////////////

const float VoxelConeTracer::ms_fVoxelTreeSize =
	static_cast<float>(ms_nVoxelTreeSize);

//////////////////////////////////////////////////////////////////////////

DebugOctreeVertex::DebugOctreeVertex()
	: vPos(IOE::Maths::g_vIdentity)
{
	arrOffset[0] = 0.0f;
	arrOffset[1] = 0.0f;
	arrOffset[2] = 0.0f;
}

//////////////////////////////////////////////////////////////////////////

DebugOctreeVertex::DebugOctreeVertex(const IOEVector &vPos_, float fX,
									 float fY, float fZ)
	: vPos(vPos_)
{
	arrOffset[0] = fX;
	arrOffset[1] = fY;
	arrOffset[2] = fZ;
}

//////////////////////////////////////////////////////////////////////////

void DebugOctreeVertex::Set(const IOEVector &vPos_, float fX, float fY,
							float fZ)
{
	vPos		 = vPos_;
	arrOffset[0] = fX;
	arrOffset[1] = fY;
	arrOffset[2] = fZ;
}

//////////////////////////////////////////////////////////////////////////

VoxelConeTracer::VoxelConeTracer()
	: m_tVoxelDepthTarget(IOE::Renderer::ETextureFormat::D24_UNORM_S8_UINT,
						  ms_fVoxelTreeSize, ms_fVoxelTreeSize)
	, m_tVertexBuffer(ms_nVoxelTreeSize * ms_nVoxelTreeSize *
					  ms_nVoxelTreeSize)
	, m_bDrawDebug(false)
	, m_fLod(0.0f)
	, m_eDirection((EAxisDirection::Enum)0)
	, m_pClampSampler(std::make_shared<Renderer::IOESamplerState>(
		  Renderer::IOESamplerState::Default()))
{
	for (int32_t nDirection(0); nDirection < EAxisDirection::NumDirections;
		 ++nDirection)
	{
		m_tVoxelRenderTarget[nDirection] =
			std::make_shared<Renderer::IOETexture>(
				Renderer::ETextureFormat::B8G8R8A8_UNORM, ms_fVoxelTreeSize,
				ms_fVoxelTreeSize);
		m_pVoxelRenderTargetTextureBind[nDirection] =
			std::make_shared<Renderer::IOETexture>(
				Renderer::ETextureFormat::B8G8R8A8_UNORM, ms_fVoxelTreeSize,
				ms_fVoxelTreeSize);
	}
	m_tVertexBuffer.SetBufferDescription(Renderer::EBufferDescription::Vertex);

	m_pClampSampler->tData.eAddressModeU =
		Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.eAddressModeV =
		Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.eAddressModeW =
		Renderer::ESamplerAddressMode::Border;
	m_pClampSampler->tData.vBorderColour =
		Maths::ColourList::Black.WithW(0.0f);
}

//////////////////////////////////////////////////////////////////////////

IOE::Renderer::ETextureFormat eFormat =
	IOE::Renderer::ETextureFormat::R32G32B32A32_FLOAT;

void VoxelConeTracer::Initialise()
{
	m_pBuildOctreeMaterial = std::make_shared<Renderer::IOEMaterial>();

#if (ENABLE_SHADOW_MAPPING == 1)
#if (DUAL_PARABALOID_SHADOW_MAPPING == 1)
	m_pBuildOctreeMaterial->AddDefinition("SHADOW_MAPPING", "2");
#else
	m_pBuildOctreeMaterial->AddDefinition("SHADOW_MAPPING", "1");
#endif
#endif
	m_pBuildOctreeMaterial->LoadMaterial(IOE::Core::Algorithm::JoinPath(
		Core::g_pEngine->GetMediaPath(), L"Materials",
		L"GenerateOctree.ioe.xml"));

	m_pDebugOctreeMaterial = std::make_shared<Renderer::IOEMaterial>(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(),
								  L"Materials", L"DebugOctree.ioe.xml"));
	m_pDebugOctreePointsMaterial = std::make_shared<Renderer::IOEMaterial>(
		Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(),
								  L"Materials", L"DebugDrawPoints.ioe.xml"));
	for (int32_t nIdx(0); nIdx < EAxisDirection::NumDirections; ++nIdx)
	{
		std::stringstream sstream;
		sstream << nIdx;
		m_pGenerateMipsMaterial[nIdx] =
			std::make_shared<Renderer::IOEMaterial>();
		m_pGenerateMipsMaterial[nIdx]->AddDefinition("DIRECTION",
													 sstream.str());
		m_pGenerateMipsMaterial[nIdx]->LoadMaterial(
			Core::Algorithm::JoinPath(Core::g_pEngine->GetMediaPath(),
									  L"Materials", L"GenerateMips.ioe.xml"));
	}

	UpdateLightSettings();

	for (std::int32_t nIdx(0); nIdx < EAxisDirection::NumDirections; ++nIdx)
	{
		m_arrUAVs[nIdx][0][0] = std::make_shared<IOE::Renderer::IOETexture>(
			eFormat, ms_fVoxelTreeSize, ms_fVoxelTreeSize, ms_fVoxelTreeSize);
		m_arrUAVs[nIdx][0][0]->SetMipToMap(0);
		m_arrUAVs[nIdx][0][0]->SetNumMips(
			MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize) + 1);
		m_arrUAVs[nIdx][0][0]->SetBufferAccess(
			Renderer::ECPUAccess::GPUReadWrite);
		m_arrUAVs[nIdx][0][0]->SetBufferDescription(
			Renderer::EBufferDescription::UnorderedAccess |
			Renderer::EBufferDescription::ShaderResource);
		m_arrUAVs[nIdx][0][0]->Setup();

		m_arrTextures[nIdx] = std::make_shared<IOE::Renderer::IOETexture>(
			eFormat, ms_fVoxelTreeSize, ms_fVoxelTreeSize, ms_fVoxelTreeSize);
		m_arrTextures[nIdx]->SetBufferDescription(
			Renderer::EBufferDescription::ShaderResource);
		m_arrTextures[nIdx]->SetNumMips(
			MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize) + 1);
		m_arrTextures[nIdx]->SetMipToMap(0);
		m_arrTextures[nIdx]->SetBufferAccess(
			Renderer::ECPUAccess::GPUReadWrite);
		m_arrTextures[nIdx]->CopyTexture(*m_arrUAVs[nIdx][0][0]);

		for (int32_t nMip(0);
			 nMip < MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize) + 1; ++nMip)
		{
			float fVoxelSize = static_cast<float>(ms_nVoxelTreeSize >> nMip);

			if (nMip > 0)
			{
				m_arrUAVs[nIdx][nMip][0] =
					std::make_shared<IOE::Renderer::IOETexture>(
						eFormat, fVoxelSize, fVoxelSize, fVoxelSize);
				m_arrUAVs[nIdx][nMip][0]->SetMipToMap(nMip);
				m_arrUAVs[nIdx][nMip][0]->SetBufferAccess(
					Renderer::ECPUAccess::GPUReadWrite);
				m_arrUAVs[nIdx][nMip][0]->SetBufferDescription(
					Renderer::EBufferDescription::UnorderedAccess |
					Renderer::EBufferDescription::ShaderResource);
				m_arrUAVs[nIdx][nMip][0]->CopyTexture(*m_arrUAVs[nIdx][0][0]);
			}

			Renderer::IOERPI::GetSingletonPtr()->ClearUAV(
				m_arrUAVs[nIdx][nMip][0].get(),
				IOE::Maths::IOEVector(0.0f, 0.0f, 0.0f, 0.0f));

			m_arrUAVs[nIdx][nMip][1] =
				std::make_shared<IOE::Renderer::IOETexture>(
					eFormat, fVoxelSize, fVoxelSize, fVoxelSize);
			m_arrUAVs[nIdx][nMip][1]->SetMipToMap(nMip);
			m_arrUAVs[nIdx][nMip][1]->SetNumMips(1);
			m_arrUAVs[nIdx][nMip][1]->SetBufferAccess(
				Renderer::ECPUAccess::GPUReadWrite);
			m_arrUAVs[nIdx][nMip][1]->SetBufferDescription(
				Renderer::EBufferDescription::ShaderResource);
			m_arrUAVs[nIdx][nMip][1]->CopyTexture(*m_arrUAVs[nIdx][0][0]);

#if (IOE_SHADER_DEBUG == IOE_ON)
			m_arrUAVs[nIdx][nMip][2] =
				std::make_shared<IOE::Renderer::IOETexture>(
					eFormat, fVoxelSize, fVoxelSize);
			m_arrUAVs[nIdx][nMip][2]->SetMipToMap(0);
			m_arrUAVs[nIdx][nMip][2]->SetNumMips(1);
			m_arrUAVs[nIdx][nMip][2]->SetBufferAccess(
				Renderer::ECPUAccess::GPUReadWrite);
			m_arrUAVs[nIdx][nMip][2]->SetBufferDescription(
				Renderer::EBufferDescription::UnorderedAccess |
				Renderer::EBufferDescription::ShaderResource);
			m_arrUAVs[nIdx][nMip][2]->Setup();

			Renderer::IOERPI::GetSingletonPtr()->ClearUAV(
				m_arrUAVs[nIdx][nMip][2].get(),
				IOE::Maths::IOEVector(0.0f, 0.0f, 0.0f, 1.0f));

			m_arrUAVs[nIdx][nMip][3] =
				std::make_shared<IOE::Renderer::IOETexture>(
					eFormat, fVoxelSize, fVoxelSize);
			m_arrUAVs[nIdx][nMip][3]->SetMipToMap(0);
			m_arrUAVs[nIdx][nMip][3]->SetNumMips(1);
			m_arrUAVs[nIdx][nMip][3]->SetBufferAccess(
				Renderer::ECPUAccess::GPUReadWrite);
			m_arrUAVs[nIdx][nMip][3]->SetBufferDescription(
				Renderer::EBufferDescription::ShaderResource);
			m_arrUAVs[nIdx][nMip][3]->CopyTexture(*m_arrUAVs[nIdx][nMip][2]);

			m_arrUAVs[nIdx][nMip][4] =
				std::make_shared<IOE::Renderer::IOETexture>(
					IOE::Renderer::ETextureFormat::R32_UINT, fVoxelSize,
					fVoxelSize);
			m_arrUAVs[nIdx][nMip][4]->SetMipToMap(0);
			m_arrUAVs[nIdx][nMip][4]->SetNumMips(1);
			m_arrUAVs[nIdx][nMip][4]->SetBufferAccess(
				Renderer::ECPUAccess::GPUReadWrite);
			m_arrUAVs[nIdx][nMip][4]->SetBufferDescription(
				Renderer::EBufferDescription::UnorderedAccess |
				Renderer::EBufferDescription::ShaderResource);
			m_arrUAVs[nIdx][nMip][4]->Setup();
#endif
		}
	}

	IOEAABB tAABB(
		CTestApplication::GetSingletonPtr()->GetMainModel()->GetAABB());
	m_vOctreeScale = g_vOneVector / (tAABB.m_vBounds[1] - tAABB.m_vBounds[0]);
	float fX, fY, fZ;
	m_vOctreeScale.Get(fX, fY, fZ);
	m_vOctreeScale = IOEVector(Maths::Min(fX, Maths::Min(fY, fZ)));

	m_vOctreeOffset = -tAABB.m_vBounds[0];
	m_vOctreeOffset.SetW(1.0f);

	float fStep		  = 1.0f / ms_fVoxelTreeSize;
	m_vInvOctreeScale = IOEVector(1.0f) / m_vOctreeScale;

	uint32_t uIdx(0);
	for (float fZ(0.5f * fStep); fZ < 1.0f; fZ += fStep)
	{
		for (float fY(0.5f * fStep); fY < 1.0f; fY += fStep)
		{
			for (float fX(0.5f * fStep); fX < 1.0f; fX += fStep)
			{
				m_tVertexBuffer.SetEmplace(
					uIdx++, IOEVector::Multiply(IOEVector(fX, fY, fZ),
												m_vInvOctreeScale) -
						m_vOctreeOffset,
					fX, fY, fZ);
			}
		}
	}

	m_pClampSampler->Setup(Renderer::IOERPI::GetSingletonPtr());

	m_pDebugOctreeMaterial->MapCBuffer("RarelyUsed");
	m_pDebugOctreeMaterial->SetShaderVariableValue("RarelyUsed", "cubeScale",
												   m_vInvOctreeScale.GetX() /
													   ms_fVoxelTreeSize);
	m_pDebugOctreeMaterial->UnmapCBuffer("RarelyUsed");

	m_pBuildOctreeMaterial->MapCBuffer("RarelyUsed");
	m_pBuildOctreeMaterial->SetShaderVariableValue("RarelyUsed", "OctreeSize",
												   ms_fVoxelTreeSize);
	m_pBuildOctreeMaterial->SetShaderVariableValue("RarelyUsed", "Near", 0.5f);
	m_pBuildOctreeMaterial->SetShaderVariableValue("RarelyUsed", "Far",
												   1000.0f);
	m_pBuildOctreeMaterial->SetSamplerVariable("g_ShadowSampler",
											   m_pClampSampler.get());
	m_pBuildOctreeMaterial->UnmapCBuffer("RarelyUsed");

	m_tVoxelDepthTarget.SetBufferAccess(Renderer::ECPUAccess::GPUReadWrite);
	m_tVoxelDepthTarget.SetBufferDescription(
		Renderer::EBufferDescription::DepthStencil);
	m_tVoxelDepthTarget.Setup();

	for (std::int32_t nIdx(0); nIdx < EAxisDirection::NumDirections; ++nIdx)
	{
		m_tVoxelRenderTarget[nIdx]->SetBufferAccess(
			Renderer::ECPUAccess::GPUReadWrite);
		m_tVoxelRenderTarget[nIdx]->SetBufferDescription(
			Renderer::EBufferDescription::RenderTarget |
			Renderer::EBufferDescription::ShaderResource);
		m_tVoxelRenderTarget[nIdx]->Setup();

		m_pVoxelRenderTargetTextureBind[nIdx]->SetBufferDescription(
			Renderer::EBufferDescription::ShaderResource);
		m_pVoxelRenderTargetTextureBind[nIdx]->CopyTexture(
			*m_tVoxelRenderTarget[nIdx]);
	}

	m_tVertexBuffer.Setup(Renderer::IOERPI::GetSingletonPtr());

	UpdateLightSettings();

	ModifyLOD(0.0f);
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::PreRender(IOE::Renderer::IOERPI *RPI)
{
	RPI->SetRasterizerState(
		CTestApplication::GetSingletonPtr()->GetNoClipRasterizerState());
	RPI->RemoveViewport(0);
	RPI->AddViewport(0.0f, 0.0f, ms_fVoxelTreeSize, ms_fVoxelTreeSize);
	RPI->SetupViewports();
	RPI->SetDepthTarget(m_tVoxelDepthTarget, false);

	// Assign materials on all models to the build octree materials
	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(RPI, m_pBuildOctreeMaterial);
	}
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::Render(IOE::Renderer::IOERPI *RPI)
{
	RenderStaticLighting(RPI);
	CreateMips(RPI);
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::PostRender(IOE::Renderer::IOERPI *RPI)
{
	RPI->SetDepthTarget(RPI->GetDefaultDepthStencilbuffer(), false);
	RPI->SetRenderTarget(RPI->GetDefaultColourBuffer());
	RPI->ResetViewports();
	RPI->SetupViewports();
	RPI->Clear();

	RPI->SetRasterizerState(
		CTestApplication::GetSingletonPtr()->GetDefaultRasterizerState());

	if (CTestApplication::GetSingletonPtr()->GetDebugDrawMode() ==
		EDebugDrawMode::DrawVoxelScene)
	{
		DebugDrawScene(RPI);
	}

	// Assign materials on all models to the generic material building
	auto pMaterial(CTestApplication::GetSingletonPtr()->GetActiveMaterial());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosX", m_arrUAVs[0][0][1].get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegX", m_arrUAVs[1][0][1].get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosY", m_arrUAVs[2][0][1].get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegY", m_arrUAVs[3][0][1].get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosZ", m_arrUAVs[4][0][1].get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegZ", m_arrUAVs[5][0][1].get());

	IOEMatrix worldToVoxelTex = IOEMatrix::Translation(m_vOctreeOffset) *
		IOEMatrix::Scale(m_vOctreeScale);

	pMaterial->MapCBuffer("VoxelConeTracing");
	pMaterial->SetShaderVariableValue("VoxelConeTracing", "VoxelSize",
									  1.0f / ms_fVoxelTreeSize);
	pMaterial->SetShaderVariableValue("VoxelConeTracing", "VoxelSizeInv",
									  ms_fVoxelTreeSize);
	pMaterial->SetShaderVariableValue("VoxelConeTracing", "WorldToVoxel",
									  worldToVoxelTex);
	pMaterial->UnmapCBuffer("VoxelConeTracing");

	for (int32_t nIdx(0);
		 nIdx < Renderer::IOEModelManager::GetSingletonPtr()->GetNumModels();
		 ++nIdx)
	{
		Renderer::IOEModelManager::GetSingletonPtr()
			->GetModel(nIdx)
			->SetMaterial(RPI, pMaterial);
	}

	/*CTestApplication::GetSingletonPtr()->CopyFullscreenTarget(
		RPI,
			m_arrUAVs[(int32_t)m_eDirection][Maths::Max((int32_t)m_fLod, / *1*
	   /0)][3]);*/
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::FinishRender(IOE::Renderer::IOERPI *RPI)
{
	IOE::Renderer::IOEMaterial *pMaterial(
		CTestApplication::GetSingletonPtr()->GetActiveMaterial().get());
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosX", nullptr);
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegX", nullptr);
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosY", nullptr);
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegY", nullptr);
	pMaterial->SetTextureVariable(RPI, "VoxelOctreePosZ", nullptr);
	pMaterial->SetTextureVariable(RPI, "VoxelOctreeNegZ", nullptr);

	if (m_bDrawDebug)
	{
		DebugDrawScene(RPI);
	}
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::RenderStaticLighting(IOE::Renderer::IOERPI *RPI)
{
	IOEMatrix worldToUnitCube(
		IOEMatrix::Translation(m_vOctreeOffset) *
		IOEMatrix::Scale(m_vOctreeScale) *
		IOEMatrix::Scale(IOEVector(2.0f, 2.0f, 2.0f)) *
		IOEMatrix::Translation(IOEVector(-1.0f, -1.0f, -1.0f)));

	for (int32_t nIdx(0); nIdx < EAxisDirection::NumDirections; ++nIdx)
	{
		RPI->SetRenderTarget(*m_tVoxelRenderTarget[nIdx]);

		RPI->Clear(IOEVector(0.0f, 0.0f, 0.0f, 1.0f));

		RPI->SetUAV(*m_arrUAVs[nIdx][0][0], 1,
					Renderer::EUAVSet::OutputMerger);
		RPI->ClearUAV(m_arrUAVs[nIdx][0][0].get(), 0);

		IOE::Maths::IOEMatrix mWorld(IOE::Maths::g_mIdentity);

		IOEMatrix unitCubeToNDC = IOEMatrix::LookAt(
			g_vIdentity, g_arrAxes[nIdx], (nIdx / 2) == 1
				? (nIdx % 2 == 0 ? g_vForwardVector : -g_vForwardVector)
				: g_vUpVector);
		IOE::Maths::IOEMatrix worldToNDC = worldToUnitCube * unitCubeToNDC;
		IOE::Maths::IOEMatrix ndcToTex   = unitCubeToNDC.Transpose();

		IOE::Maths::IOEMatrix mInvXPose(mWorld.Inverse());
		mInvXPose.TransposeSet();

		m_pBuildOctreeMaterial->MapCBuffer("GlobalSettings");
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"GlobalSettings", "WorldViewProj", worldToNDC);
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"GlobalSettings", "WorldInvTranspose", mInvXPose);
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"GlobalSettings", "WorldToOctree", ndcToTex);
		m_pBuildOctreeMaterial->UnmapCBuffer("GlobalSettings");

		IOE::Renderer::IOEModelManager::GetSingletonPtr()->RenderAllModels(
			RPI, false);

		RPI->UnsetUAV(1, Renderer::EUAVSet::OutputMerger);
	}
	m_pBuildOctreeMaterial->SetTextureVariable(RPI, "ShadowMap", nullptr);
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::CreateMips(IOE::Renderer::IOERPI *RPI)
{
	for (int32_t nDirection(0); nDirection < EAxisDirection::NumDirections;
		 ++nDirection)
	{
		m_pGenerateMipsMaterial[nDirection]->Set(RPI, false);

		for (int32_t nMip(0); nMip <= MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize);
			 ++nMip)
		{
			auto pSrc(
				m_arrUAVs[nDirection][nMip == 0 ? 0 : (nMip - 1)][1].get());
			auto pDest(m_arrUAVs[nDirection][nMip == 0 ? 1 : nMip][0].get());

			m_pGenerateMipsMaterial[nDirection]->SetTextureVariable(
				RPI, "SrcBuffer", pSrc);

			m_pGenerateMipsMaterial[nDirection]->MapCBuffer("Globals");
			m_pGenerateMipsMaterial[nDirection]->SetShaderVariableValue(
				"Globals", "OctreeSize",
				static_cast<float>(ms_nVoxelTreeSize >> nMip));
			m_pGenerateMipsMaterial[nDirection]->SetShaderVariableValue(
				"Globals", "Offset", nMip == 0 ? 1.0f : 2.0f);
			m_pGenerateMipsMaterial[nDirection]->UnmapCBuffer("Globals");

			RPI->SetUAV(*pDest, 0, Renderer::EUAVSet::ComputeShader);

#if (IOE_SHADER_DEBUG == IOE_ON)
			auto pRender2D(m_arrUAVs[nDirection][nMip][2].get());
			RPI->SetUAV(*pRender2D, 1, Renderer::EUAVSet::ComputeShader);

			auto pDepth2D(m_arrUAVs[nDirection][nMip][4].get());
			RPI->SetUAV(*pDepth2D, 2, Renderer::EUAVSet::ComputeShader);

			RPI->ClearUAV(pDepth2D, ~0U);
#endif

			int32_t nNumThreads(ms_nVoxelTreeSize >> nMip);
			m_pGenerateMipsMaterial[nDirection]->Dispatch(
				RPI, nNumThreads, nNumThreads, nNumThreads);

#if (IOE_SHADER_DEBUG == IOE_ON)
			RPI->UnsetUAV(2, Renderer::EUAVSet::ComputeShader);
			RPI->UnsetUAV(1, Renderer::EUAVSet::ComputeShader);
#endif

			RPI->UnsetUAV(0, Renderer::EUAVSet::ComputeShader);

			m_pGenerateMipsMaterial[nDirection]->SetTextureVariable(
				RPI, "SrcBuffer", nullptr);
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::DebugDrawScene(Renderer::IOERPI *RPI)
{
	m_pDebugOctreeMaterial->Set(RPI);

	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler0",
											   m_arrTextures[0].get());
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler1",
											   m_arrTextures[1].get());
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler2",
											   m_arrTextures[2].get());
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler3",
											   m_arrTextures[3].get());
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler4",
											   m_arrTextures[4].get());
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler5",
											   m_arrTextures[5].get());

	Maths::IOECamera &rCamera(RPI->GetCamera());
	m_pDebugOctreeMaterial->MapCBuffer("Globals");
	m_pDebugOctreeMaterial->SetShaderVariableValue(
		"Globals", "WorldViewProj",
		rCamera.GetViewMatrix() * rCamera.GetProjectionMatrix());
	m_pDebugOctreeMaterial->UnmapCBuffer("Globals");

	RPI->DrawBuffer(&m_tVertexBuffer, Renderer::EPrimitiveTopology::Point);

	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler0", nullptr);
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler1", nullptr);
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler2", nullptr);
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler3", nullptr);
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler4", nullptr);
	m_pDebugOctreeMaterial->SetTextureVariable(RPI, "sampler5", nullptr);
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::ModifyLOD(float fDifference)
{
	m_fLod = IOE::Maths::Clamp(
		m_fLod + fDifference, 0.0f,
		static_cast<float>(MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize)));
	m_pDebugOctreeMaterial->MapCBuffer("Test");
	m_pDebugOctreeMaterial->SetShaderVariableValue("Test", "lod", m_fLod);
	m_pDebugOctreeMaterial->UnmapCBuffer("Test");
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::ModifyAxis(int32_t nDiff)
{
	m_eDirection =
		(EAxisDirection::Enum)(((int32_t)m_eDirection + nDiff) %
							   (int32_t)EAxisDirection::NumDirections);
	while (m_eDirection < 0)
	{
		m_eDirection = (EAxisDirection::Enum)(
			(int32_t)m_eDirection + (int32_t)EAxisDirection::NumDirections);
	}
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::SetShadowMap(IOE::Renderer::IOETexture *pShadowMap)
{
	m_pBuildOctreeMaterial->SetTextureVariable(IOE::Renderer::IOERPI::GetSingletonPtr(), "ShadowMap", pShadowMap);
}

//////////////////////////////////////////////////////////////////////////

void VoxelConeTracer::UpdateLightSettings()
{
	if (m_pBuildOctreeMaterial)
	{
		m_pBuildOctreeMaterial->MapCBuffer("LightSettings");
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "LightPos",
			CTestApplication::GetSingletonPtr()->GetLightPosition());
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "LightColour",
			CTestApplication::GetSingletonPtr()->GetLightColour());
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "Intensity",
			CTestApplication::GetSingletonPtr()->GetLightIntensity());
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "Roughness",
			CTestApplication::GetSingletonPtr()->GetRoughness());
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "LightSpecular",
			CTestApplication::GetSingletonPtr()->GetSpecular());
		m_pBuildOctreeMaterial->SetShaderVariableValue(
			"LightSettings", "LightRadius",
			CTestApplication::GetSingletonPtr()->GetLightRadius());
		m_pBuildOctreeMaterial->UnmapCBuffer("LightSettings");
	}
}

//////////////////////////////////////////////////////////////////////////