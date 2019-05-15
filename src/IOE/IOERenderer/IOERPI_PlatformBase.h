#pragma once

#include <vector>

#include "IOEComSharedPtr.h"
#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOERenderer/IOETexture.h>

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOERPI;
	class IOEBufferBase;
	class IOEVertexFormat;
	enum class EPrimitiveTopology;
	enum class EUAVSet;
	struct IOERasterizerState;
	struct IOEBlendState;
	struct IOESamplerState;
	struct IOEDepthState;

	//////////////////////////////////////////////////////////////////////////

	enum class EProfileMarkerType
	{
		Start,
		End,
		Single
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOERasterizerState_PlatformBase
	{
		IOERasterizerState_PlatformBase(IOERasterizerState &rState)
			: m_rState(rState)
		{
		}

		virtual void Setup(IOERPI *pRPI) = 0;

	protected:
		IOERasterizerState &GetState()
		{
			return m_rState;
		}
		const IOERasterizerState &GetState() const
		{
			return m_rState;
		}

	private:
		IOERasterizerState &m_rState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEBlendState_PlatformBase
	{
		IOEBlendState_PlatformBase(IOEBlendState &rState)
			: m_rState(rState)
		{
		}

		virtual void Setup(IOERPI *pRPI) = 0;

	protected:
		IOEBlendState &GetState()
		{
			return m_rState;
		}
		const IOEBlendState &GetState() const
		{
			return m_rState;
		}

	private:
		IOEBlendState &m_rState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOESamplerState_PlatformBase
	{
		IOESamplerState_PlatformBase(IOESamplerState &rState)
			: m_rState(rState)
		{
		}

		virtual void Setup(IOERPI *pRPI) = 0;

	protected:
		IOESamplerState &GetState()
		{
			return m_rState;
		}
		const IOESamplerState &GetState() const
		{
			return m_rState;
		}

	private:
		IOESamplerState &m_rState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEDepthState_PlatformBase
	{
		IOEDepthState_PlatformBase(IOEDepthState &rState)
			: m_rState(rState)
		{
		}

		virtual void Setup(IOERPI *pRPI) = 0;

	protected:
		IOEDepthState &GetState()
		{
			return m_rState;
		}
		const IOEDepthState &GetState() const
		{
			return m_rState;
		}

	private:
		IOEDepthState &m_rState;
	};

	//////////////////////////////////////////////////////////////////////////

	interface_class IOERPI_PlatformBase
	{
	public:
		IOERPI_PlatformBase()
			: m_bSoftwareMode(false)
		{
		}

		void SetRPI(weak_ptr<IOERPI> pRPI)
		{
			m_pRPI = pRPI;
		}

		virtual void ClearDepth(IOE::Maths::IOEVector & vColour)											= 0;
		virtual void Clear(IOE::Maths::IOEVector & vColour)													= 0;
		virtual void ClearUAV(IOETexture * pUAV, IOE::Maths::IOEVector &vColour = Maths::ColourList::Black) = 0;
		virtual void ClearUAV(IOETexture * pUAV, std::uint32_t uARGB)										= 0;
		virtual void ResetUAVs()																			= 0;
		virtual void ClearBuffer(IOETexture * pBuffer, const void *pSrcValue, std::size_t uSrcByteSize)		= 0;

		virtual void OnInit(weak_ptr<IOERPI> pRPI)
		{
			m_pRPI = pRPI;
		}
		virtual void Resize(float fNewWidth, float fNewHeight) = 0;
		virtual void SetupViewports()						   = 0;

		virtual void StartRender()  = 0;
		virtual void FinishRender() = 0;

		virtual void ResetState() = 0;

		virtual void SetRasterizerState(const IOERasterizerState &rhs)									   = 0;
		virtual void SetBlendState(const IOEBlendState &rhs, std::uint32_t uWriteMask = 0xFFFFFFFF,
								   const IOE::Maths::IOEVector &srrBlendFactor = IOE::Maths::g_vOneVector) = 0;
		virtual void SetSamplerState(const IOESamplerState &rhs)										   = 0;
		virtual void SetDepthState(const IOEDepthState &rhs, std::uint32_t uStencilRef = 0)				   = 0;

		virtual void UnsetRenderTarget(int32_t nRenderTarget = 0)											= 0;
		virtual void SetRenderTarget(IOE::Renderer::IOETexture & rTexture, int32_t nRenderTargetIdx = 0,
									 bool bSet = true)														= 0;
		virtual void SetRenderTargets(std::vector<IOE::Renderer::IOETexture> arrTextures, bool bSet = true) = 0;

		virtual void UnsetUAV(int32_t nUAV, EUAVSet bSet)									   = 0;
		virtual void SetUAV(IOE::Renderer::IOETexture & rTexture, int32_t nUAVIdx, EUAVSet bSet,
							int arrOffset = 0)												   = 0;
		virtual void SetUAVs(std::vector<IOE::Renderer::IOETexture> arrTextures, EUAVSet bSet) = 0;

		virtual void SetDepthTarget(IOE::Renderer::IOETexture & rDepthTexture, bool bSet = true) = 0;

		virtual void DrawBuffer(IOEBufferBase * pVertexBuffer, EPrimitiveTopology eTopology,
								IOEBufferBase *pIndexBuffer = nullptr, std::int32_t nNumElements = -1) = 0;
		virtual void SetPrimitiveTopology(EPrimitiveTopology eTopology)								   = 0;

		virtual void SetProfileMarker(const std::string &szProfilerMark, EProfileMarkerType eProfileMarker,
									  const IOE::Maths::IOEVector &vColour) = 0;

	public:
		void SetInSoftwareMode(bool bSoftwareMode)
		{
			m_bSoftwareMode = bSoftwareMode;
		}

	protected:
		bool IsInSoftwareMode() const
		{
			return m_bSoftwareMode;
		}

		weak_ptr<IOERPI> GetRPIShared()
		{
			return m_pRPI;
		}
		const weak_ptr<IOERPI> GetRPIShared() const
		{
			return m_pRPI;
		}

		IOERPI *GetRPI()
		{
			return m_pRPI.lock().get();
		}

		const IOERPI *GetRPI() const
		{
			return m_pRPI.lock().get();
		}

	private:
		weak_ptr<IOERPI> m_pRPI;
		bool m_bSoftwareMode;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE