
#include <fstream>
#include <string>

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOECore/IOEManagerContainer.h>

#include "IOERPI.h"

#include <IOE/IOEApplication/IOEApplication.h>

#include "IOEModel.h"
#include "IOEModelManager.h"
#include "IOEShader.h"

//////////////////////////////////////////////////////////////////////////

using std::wstring;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOERPI::IOERPI()
		: IOEManager("IOERPI")
		, m_tPlatform()
		, m_tDefaultViewport(IOE::Application::GetDefaultResX<float>(), IOE::Application::GetDefaultResY<float>())
		, m_tDefaultColourBuffer(ETextureFormat::B8G8R8A8_UNORM, IOE::Application::GetDefaultResX<float>(),
								 IOE::Application::GetDefaultResY<float>(), 1, true)
		, m_tDefaultDepthStencil(ETextureFormat::D24_UNORM_S8_UINT, IOE::Application::GetDefaultResX<float>(),
								 IOE::Application::GetDefaultResY<float>())
		, m_uNumDebugPoints(0)
		, m_uNumDebugLines(0)
		, m_uNumDebugTriangles(0)
		, m_tRasterizerState(IOERasterizerState::Default())
		, m_tBlendState(IOEBlendState::Default())
		, m_tDefaultSamplerState(IOESamplerState::Default())
		, m_tDefaultDepthState(IOEDepthState::Default())
		, m_eTopology(EPrimitiveTopology::TriangleList)
	{
		m_tDefaultColourBuffer.SetBufferDescription(EBufferDescription::RenderTarget);
		m_tDefaultDepthStencil.SetBufferDescription(EBufferDescription::DepthStencil);

		IOE::Maths::IOECamera::CCameraProperties tCameraProperties;
		tCameraProperties.fAspect = IOE::Application::GetDefaultResX<float>(),
		IOE::Application::GetDefaultResY<float>();
		tCameraProperties.fFar  = 1000.0f;
		tCameraProperties.fNear = 0.5f;
		tCameraProperties.fFOV  = 90.0f;
		m_mDefaultCamera.SetCameraProperties(tCameraProperties);

		m_uCurrentMaterialIndex = IOEMaterial::INVALID_RESOURCE_INDEX;
		for (int32_t nIdx(0); nIdx < EShaderType::NumShaders; ++nIdx)
		{
			m_arrCurrentShaderIndexes[nIdx] = IOEShader::INVALID_RESOURCE_INDEX;
		}

		m_mCamera = m_mDefaultCamera;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::OnManagerInit()
	{
		m_tPlatform.OnInit(shared_from_this());
		// m_tDefaultDepthStencil.Setup();

		wstring szMediaDir(IOE::Core::g_pEngine->GetMediaPath());
		std::wstring szPath(IOE::Core::Algorithm::JoinPath(szMediaDir, L"Materials", L"DebugDraw.ioe.xml"));

		m_pDebugDrawShader = std::make_shared<IOEMaterial>(szPath);

		m_tRasterizerState.Setup(this);
		m_tBlendState.Setup(this);
		m_tDefaultSamplerState.Setup(this);
		m_tDefaultDepthState.Setup(this);
		m_tDefaultColourBuffer.Setup();
		m_tDefaultDepthStencil.Setup();

		m_tPlatform.SetPrimitiveTopology(m_eTopology);

		IOE_ASSERT(m_pDebugDrawShader != nullptr, "Failed to find debug draw shader");
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::RemoveViewport(std::int32_t nIdx)
	{
		IOE_ASSERT(nIdx < m_arrViewports.size(), "Viewport out of range!");
		m_arrViewports.erase(std::begin(m_arrViewports) + nIdx);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::AddViewport(IOEViewport tViewport)
	{
		m_arrViewports.push_back(std::move(tViewport));
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::AddViewport(float fTop, float fLeft, float fWidth, float fHeight, float fMinDepth /*=0.0f*/,
							 float fMaxDepth /*=1.0f*/)
	{
		m_arrViewports.emplace_back(fTop, fLeft, fWidth, fHeight, fMinDepth, fMaxDepth);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::ResetViewports()
	{
		m_arrViewports.clear();
		m_arrViewports.push_back(m_tDefaultViewport);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::Resize(float fNewWidth, float fNewHeight)
	{
		m_tDefaultViewport.Resize(fNewWidth, fNewHeight);
		m_tDefaultColourBuffer.Resize(fNewWidth, fNewHeight);
		m_tDefaultDepthStencil.Resize(fNewWidth, fNewHeight);

		m_tPlatform.Resize(fNewWidth, fNewHeight);

		IOE::Maths::IOECamera::CCameraProperties tCameraProperties;
		tCameraProperties.fAspect = fNewWidth / fNewHeight;
		tCameraProperties.fFar	= 1000.0f;
		tCameraProperties.fNear   = 0.5f;
		tCameraProperties.fFOV	= 90.0f;
		m_mDefaultCamera.SetCameraProperties(tCameraProperties);

		m_mCamera = m_mDefaultCamera;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::OnPreRender(IOE::Renderer::IOERPI *pPtr)
	{
		ResetState();

		ResetViewports();
		SetCamera(m_mDefaultCamera);
		SetDepthTarget(m_tDefaultDepthStencil, false);
		SetRenderTarget(m_tDefaultColourBuffer, 0, true);
		SetRasterizerState(m_tRasterizerState);
		SetBlendState(m_tBlendState);
		SetDepthState(m_tDefaultDepthState);

		Clear();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::OnRender(IOE::Renderer::IOERPI *pPtr)
	{
		m_tPlatform.StartRender();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::OnPostRender(IOE::Renderer::IOERPI *pPtr)
	{
		m_tPlatform.FinishRender();

		FlushDebugBuffers();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetRenderTarget(IOE::Renderer::IOETexture &rRenderTargets, int32_t m_nRenderTargetIdx /*=0*/,
								 bool bSet /*=true*/)
	{
		m_tPlatform.SetRenderTarget(rRenderTargets, m_nRenderTargetIdx, bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetDepthTarget(IOE::Renderer::IOETexture &rDepthTexture, bool bSet /*=true*/)
	{
		m_tPlatform.SetDepthTarget(rDepthTexture, bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetRenderTargets(std::vector<IOE::Renderer::IOETexture> arrRenderTargets, bool bSet /*=true*/)
	{
		m_tPlatform.SetRenderTargets(arrRenderTargets, bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::ResetUAVs()
	{
		m_tPlatform.ResetUAVs();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetUAVs(std::vector<IOE::Renderer::IOETexture> arrUAVs, EUAVSet bSet /*= EUAVSet::OutputMerger*/)
	{
		m_tPlatform.SetUAVs(arrUAVs, bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetUAV(IOE::Renderer::IOETexture &rUAV, int32_t m_nUAVIdx /*=0*/,
						EUAVSet bSet /*= EUAVSet::OutputMerger*/, int arrOffset /*=0*/)
	{
		m_tPlatform.SetUAV(rUAV, m_nUAVIdx, bSet, arrOffset);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::UnsetRenderTarget(int32_t nRenderTarget /*=0*/)
	{
		m_tPlatform.UnsetRenderTarget(nRenderTarget);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::UnsetUAV(int32_t nUAV /*=0*/, EUAVSet bSet /*= EUAVSet::OutputMerger*/)
	{
		m_tPlatform.UnsetUAV(nUAV, bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::ResetState()
	{
		m_tPlatform.ResetState();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetupViewports()
	{
		m_tPlatform.SetupViewports();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::UnsetTextureSampler(std::string szName)
	{
		for (auto tIt(std::begin(m_arrTextureSamplers)); tIt != std::end(m_arrTextureSamplers); ++tIt)
		{
			if ((*tIt).first == szName)
			{
				tIt = m_arrTextureSamplers.erase(tIt);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::SetTextureSampler(std::string szName, IOESamplerState *pSampler)
	{
		for (auto tIt(std::begin(m_arrTextureSamplers)); tIt != std::end(m_arrTextureSamplers); ++tIt)
		{
			if ((*tIt).first == szName)
			{
				(*tIt).second = pSampler;
				return;
			}
		}
		m_arrTextureSamplers.emplace_back(szName, pSampler);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI::AddDebugBox(const IOEVector &vCentre, const IOEVector &vExtents,
							 const IOEVector &vColour /*=Maths::ColourList::Red*/)
	{
		float fXMin, fYMin, fZMin;
		float fXMax, fYMax, fZMax;

		(vCentre - vExtents).Get(fXMin, fYMin, fZMin);
		(vCentre + vExtents).Get(fXMax, fYMax, fZMax);

		AddDebugLine(IOEVector(fXMin, fYMin, fZMin), IOEVector(fXMin, fYMax, fZMin), vColour, vColour);
		AddDebugLine(IOEVector(fXMin, fYMax, fZMin), IOEVector(fXMax, fYMax, fZMin), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMax, fZMin), IOEVector(fXMax, fYMin, fZMin), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMin, fZMin), IOEVector(fXMin, fYMin, fZMin), vColour, vColour);

		// Rear face
		AddDebugLine(IOEVector(fXMin, fYMin, fZMax), IOEVector(fXMin, fYMax, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMin, fYMax, fZMax), IOEVector(fXMax, fYMax, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMax, fZMax), IOEVector(fXMax, fYMin, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMin, fZMax), IOEVector(fXMin, fYMin, fZMax), vColour, vColour);

		// Connectors
		AddDebugLine(IOEVector(fXMin, fYMin, fZMin), IOEVector(fXMin, fYMin, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMin, fYMax, fZMin), IOEVector(fXMin, fYMax, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMax, fZMin), IOEVector(fXMax, fYMax, fZMax), vColour, vColour);
		AddDebugLine(IOEVector(fXMax, fYMin, fZMin), IOEVector(fXMax, fYMin, fZMax), vColour, vColour);
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE