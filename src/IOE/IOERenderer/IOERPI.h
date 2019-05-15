#pragma once

#include <vector>
#include <array>
#include <utility>
#include <memory>
#include <limits>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEManager.h>

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEMaths/Camera.h>

#include "IOERPI_Defines.h"
#include RPI_INCLUDE(IOERPI_Platform.h)
#include <IOE/IOERenderer/IOEVertexFormat.h>
#include "IOEShader_PlatformBase.h"
#include "IOEViewport.h"
#include "IOETexture.h"

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	using IOE::Maths::IOEVector;
	using std::array;
	using std::shared_ptr;

	class IOEBufferBase;
	class IOEVertexFormat;
	class IOETexture;

	//////////////////////////////////////////////////////////////////////////

	enum class EPrimitiveTopology
	{
		Point,
		LineList,
		LineStrip,
		LineListWithAdjacency,
		LineStripWithAdjacency,
		TriangleList,
		TriangleStrip,
		TriangleListWithAdjacency,
		TriangleStripWithAdjacency
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EFillMode
	{
		Wireframe,
		Solid
	};

	enum class ECullMode
	{
		None,
		Front,
		Back
	};

	enum class EUAVSet
	{
		OutputMerger,
		ComputeShader,
		None
	};

	struct IOERasterizerState
	{
		IOERasterizerState()
			: m_tPlatform(*this)
		{
		}

		IOERasterizerState(const IOERasterizerState &rhs)
			: tData(rhs.tData)
			, m_tPlatform(*this)
		{
		}
		IOERasterizerState(IOERasterizerState &&rhs)
			: tData(std::move(rhs.tData))
			, m_tPlatform(*this)
		{
		}

		IOERasterizerState &operator=(const IOERasterizerState &rhs) = delete;
		IOERasterizerState &operator=(IOERasterizerState &&rhs) = delete;

		static IOERasterizerState Default()
		{
			IOERasterizerState tState;
			tState.tData.eFillMode				= EFillMode::Solid;
			tState.tData.eCullMode				= ECullMode::Back;
			tState.tData.bFrontCounterClockwise = false;
			tState.tData.nDepthBias				= 0;
			tState.tData.fDepthBiasClamp		= 0.0f;
			tState.tData.fSlopeScaledDepthBias  = 0.0f;
			tState.tData.bDepthClipEnable		= true;
			tState.tData.bScissorEnable			= false;
			tState.tData.bMultisampleEnable		= false;
			tState.tData.bAntialiasedLineEnable = false;

			return tState;
		}

		void Setup(IOERPI *pRPI)
		{
			m_tPlatform.Setup(pRPI);
		}

		IOERasterizerState_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOERasterizerState_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

		struct _
		{
			EFillMode eFillMode;
			ECullMode eCullMode;
			bool bFrontCounterClockwise;
			std::int32_t nDepthBias;
			float fDepthBiasClamp;
			float fSlopeScaledDepthBias;
			bool bDepthClipEnable;
			bool bScissorEnable;
			bool bMultisampleEnable;
			bool bAntialiasedLineEnable;
		} tData;
		IOERasterizerState_Platform m_tPlatform;
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EBlendValue
	{
		Zero,
		One,
		SrcColour,
		InvSrcColour,
		SrcAlpha,
		InvSrcAlpha,
		DestAlpha,
		InvDestAlpha,
		DestColour,
		InvDestColour,
		SrcAlphaSat,
		BlendFactor,
		InvBlendFactor
	};

	enum class EBlendOperation
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};

	namespace EBlendWriteChannel
	{
		enum Enum
		{
			None  = 0,
			Red   = 0x1,
			Green = 0x2,
			Blue  = 0x4,
			Alpha = 0x8,
			All = (((Red | Green) | Blue) | Alpha)
		};
	}

	struct IOEBlendState
	{
		IOEBlendState()
			: m_tPlatform(*this)
		{
		}

		IOEBlendState(const IOEBlendState &rhs)
			: tData(rhs.tData)
			, m_tPlatform(*this)
		{
		}
		IOEBlendState(IOEBlendState &&rhs)
			: tData(std::move(rhs.tData))
			, m_tPlatform(*this)
		{
		}

		static IOEBlendState Default()
		{
			IOEBlendState tState;

			tState.tData.bBlendEnable = false;

			tState.tData.eSrcBlend  = EBlendValue::One;
			tState.tData.eDestBlend = EBlendValue::Zero;
			tState.tData.eBlendOp   = EBlendOperation::Add;

			tState.tData.eSrcBlendAlpha  = EBlendValue::One;
			tState.tData.eDestBlendAlpha = EBlendValue::Zero;
			tState.tData.eBlendOpAlpha   = EBlendOperation::Add;

			tState.tData.nRenderTargetWriteMask = EBlendWriteChannel::All;

			return tState;
		}

		static IOEBlendState DefaultAlpha()
		{
			IOEBlendState tState;

			tState.tData.bBlendEnable = true;

			tState.tData.eSrcBlend  = EBlendValue::SrcAlpha;
			tState.tData.eDestBlend = EBlendValue::InvSrcAlpha;
			tState.tData.eBlendOp   = EBlendOperation::Add;

			tState.tData.eSrcBlendAlpha  = EBlendValue::Zero;
			tState.tData.eDestBlendAlpha = EBlendValue::Zero;
			tState.tData.eBlendOpAlpha   = EBlendOperation::Add;

			tState.tData.nRenderTargetWriteMask = EBlendWriteChannel::All;

			return tState;
		}

		static IOEBlendState DefaultNoWrite()
		{
			IOEBlendState tState;

			tState.tData.bBlendEnable = false;

			tState.tData.eSrcBlend  = EBlendValue::Zero;
			tState.tData.eDestBlend = EBlendValue::Zero;
			tState.tData.eBlendOp   = EBlendOperation::Add;

			tState.tData.eSrcBlendAlpha  = EBlendValue::Zero;
			tState.tData.eDestBlendAlpha = EBlendValue::Zero;
			tState.tData.eBlendOpAlpha   = EBlendOperation::Add;

			tState.tData.nRenderTargetWriteMask = EBlendWriteChannel::None;

			return tState;
		}

		struct _
		{
			bool bBlendEnable;

			EBlendValue eSrcBlend;
			EBlendValue eDestBlend;
			EBlendOperation eBlendOp;

			EBlendValue eSrcBlendAlpha;
			EBlendValue eDestBlendAlpha;
			EBlendOperation eBlendOpAlpha;

			std::uint8_t nRenderTargetWriteMask;
		} tData;

		IOEBlendState_Platform m_tPlatform;

		void Setup(IOERPI *pRPI)
		{
			m_tPlatform.Setup(pRPI);
		}

		IOEBlendState_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOEBlendState_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ESamplerFilter
	{
		MinMagPointMipLinear,
		MinPointMagLinearMipPoint,
		MinPointMagMipLinear,
		MinLinearMagMipPoint,
		MinLinearMagPointMipLinear,
		MinMagLinearMipPoint,
		MinMagMipLinear,
		Anisotropic,
		ComparisonMinMagMipPoint,
		ComparisonMinMagPointMipLinear,
		ComparisonMinPointMagLinearMipPoint,
		ComparisonMinPointMagMipLinear,
		ComparisonMinLinearMagMipPoint,
		ComparisonMinLinearMagPointMipLinear,
		ComparisonMinMagLinearMipPoint,
		ComparisonMinMagMipLinear,
		ComparisonAnisotropic,
		MinimumMinMagMipPoint,
		MinimumMinMagPointMipLinear,
		MinimumMinPointMagLinearMipPoint,
		MinimumMinPointMagMipLinear,
		MinimumMinLinearMagMipPoint,
		MinimumMinLinearMagPointMipLinear,
		MinimumMinMagLinearMipPoint,
		MinimumMinMagMipLinear,
		MinimumAnisotropic,
		MaximumMinMagMipPoint,
		MaximumMinMagPointMipLinear,
		MaximumMinPointMagLinearMipPoint,
		MaximumMinPointMagMipLinear,
		MaximumMinLinearMagMipPoint,
		MaximumMinLinearMagPointMipLinear,
		MaximumMinMagLinearMipPoint,
		MaximumMinMagMipLinear,
		MaximumAnisotropic
	};

	enum class ESamplerAddressMode
	{
		Wrap,
		Mirror,
		Clamp,
		Border,
		MirrorOnce
	};

	enum class EComparisonFunction
	{
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};

	struct IOESamplerState
	{
	public:
		IOESamplerState()
			: m_tPlatform(*this)
		{
		}

		IOESamplerState(const IOESamplerState &rhs)
			: tData(rhs.tData)
			, m_tPlatform(*this)
		{
		}
		IOESamplerState(IOESamplerState &&rhs)
			: tData(std::move(rhs.tData))
			, m_tPlatform(*this)
		{
		}

		static IOESamplerState Default()
		{
			IOESamplerState tState;

			tState.tData.eFilter = ESamplerFilter::MinMagMipLinear;

			tState.tData.eAddressModeU = ESamplerAddressMode::Clamp;
			tState.tData.eAddressModeV = ESamplerAddressMode::Clamp;
			tState.tData.eAddressModeW = ESamplerAddressMode::Clamp;

			tState.tData.fLODBias		= 0.0f;
			tState.tData.uMaxAnisotropy = 1;
			tState.tData.eComparison	= EComparisonFunction::Never;
			tState.tData.vBorderColour  = Maths::g_vOneVector;

#undef max

			tState.tData.fMinLOD = -std::numeric_limits<float>::max();
			tState.tData.fMaxLOD = std::numeric_limits<float>::max();

			return tState;
		}

		struct _
		{
			ESamplerFilter eFilter;
			ESamplerAddressMode eAddressModeU;
			ESamplerAddressMode eAddressModeV;
			ESamplerAddressMode eAddressModeW;

			float fLODBias;
			std::uint32_t uMaxAnisotropy;
			EComparisonFunction eComparison;
			Maths::IOEVector vBorderColour;

			float fMinLOD;
			float fMaxLOD;
		} tData;

		IOESamplerState_Platform m_tPlatform;

		void Setup(IOERPI *pRPI)
		{
			m_tPlatform.Setup(pRPI);
		}

		IOESamplerState_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOESamplerState_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EStencilOperation
	{
		Keep,
		Zero,
		Replace,
		IncrementSaturate,
		DecrementSaturate,
		Invert,
		Increment,
		Decrement
	};

	struct IOEDepthState
	{
	public:
		IOEDepthState()
			: m_tPlatform(*this)
		{
		}

		IOEDepthState(const IOEDepthState &rhs)
			: tData(rhs.tData)
			, m_tPlatform(*this)
		{
		}
		IOEDepthState(IOEDepthState &&rhs)
			: tData(std::move(rhs.tData))
			, m_tPlatform(*this)
		{
		}

		static IOEDepthState Default()
		{
			IOEDepthState tState;

			tState.tData.bDepthEnable	= true;
			tState.tData.uDepthWriteMask = 1;
			tState.tData.eDepthFunc		 = EComparisonFunction::Less;

			tState.tData.bStencilEnable	= false;
			tState.tData.uStencilReadMask  = 0xff;
			tState.tData.uStencilWriteMask = 0xff;

			tState.tData.tFrontFace.eStencilFailOp = EStencilOperation::Keep;
			tState.tData.tFrontFace.eStencilDepthFailOp =
				EStencilOperation::Keep;
			tState.tData.tFrontFace.eStencilPassOp = EStencilOperation::Keep;
			tState.tData.tFrontFace.eStencilFunc = EComparisonFunction::Always;

			tState.tData.tRearFace.eStencilFailOp = EStencilOperation::Keep;
			tState.tData.tRearFace.eStencilDepthFailOp =
				EStencilOperation::Keep;
			tState.tData.tRearFace.eStencilPassOp = EStencilOperation::Keep;
			tState.tData.tRearFace.eStencilFunc = EComparisonFunction::Always;

			return tState;
		}

		IOEDepthState_Platform m_tPlatform;

		void Setup(IOERPI *pRPI)
		{
			m_tPlatform.Setup(pRPI);
		}
		IOEDepthState_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		const IOEDepthState_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

		struct _
		{
			struct _Stencil
			{
				EStencilOperation eStencilFailOp;
				EStencilOperation eStencilDepthFailOp;
				EStencilOperation eStencilPassOp;
				EComparisonFunction eStencilFunc;
			};

			bool bDepthEnable;
			std::uint32_t uDepthWriteMask;
			EComparisonFunction eDepthFunc;

			bool bStencilEnable;
			std::uint32_t uStencilReadMask;
			std::uint32_t uStencilWriteMask;

			_Stencil tFrontFace;
			_Stencil tRearFace;
		} tData;
	};

	//////////////////////////////////////////////////////////////////////////

	// This is the main interface for all rendering platform interfaces (RPI).
	// This should be a platform independent interface to control the rendering
	// pipeline.
	class IOERPI : public IOE::Core::IOEManager<IOERPI>
	{
	public:
		static const std::uint32_t ms_uNumDebugPoints	= 64 * 1024;
		static const std::uint32_t ms_uNumDebugLines	 = 256 * 1024;
		static const std::uint32_t ms_uNumDebugTriangles = 128 * 1024;

	public:
		IOERPI();

		FORCEINLINE void
		ClearDepth(IOE::Maths::IOEVector &vColour = Maths::ColourList::Black)
		{
			m_tPlatform.ClearDepth(vColour);
		}
		FORCEINLINE void
		Clear(IOE::Maths::IOEVector &vColour = Maths::ColourList::Black)
		{
			m_tPlatform.Clear(vColour);
		}
		FORCEINLINE void
		ClearUAV(IOETexture *pUAV,
				 IOE::Maths::IOEVector &vColour = Maths::ColourList::Black)
		{
			m_tPlatform.ClearUAV(pUAV, vColour);
		}
		FORCEINLINE void ClearUAV(IOETexture *pUAV, std::uint32_t uARGB)
		{
			m_tPlatform.ClearUAV(pUAV, uARGB);
		}

		// Blits rValue over the entirety of pBuffer
		template <typename _T>
		FORCEINLINE void ClearBuffer(IOETexture *pBuffer, const _T &rValue)
		{
			m_tPlatform.ClearBuffer(pBuffer, &rValue, sizeof(_T));
		}

		virtual void OnManagerInit() override final;
		virtual void OnPreRender(IOE::Renderer::IOERPI *pPtr) override final;
		virtual void OnRender(IOE::Renderer::IOERPI *pPtr) override final;
		virtual void OnPostRender(IOE::Renderer::IOERPI *pPtr) override final;

		void ResetState();

		FORCEINLINE IOERPI_Platform &GetPlatform()
		{
			return m_tPlatform;
		}
		FORCEINLINE const IOERPI_Platform &GetPlatform() const
		{
			return m_tPlatform;
		}

		void UnsetRenderTarget(int32_t nRenderTarget = 0);
		void SetRenderTargets(
			std::vector<IOE::Renderer::IOETexture> arrRenderTargets,
			bool bSet = true);
		void SetRenderTarget(IOE::Renderer::IOETexture &rRenderTargets,
							 int32_t m_nRenderTargetIdx = 0, bool bSet = true);
		void SetDepthTarget(IOE::Renderer::IOETexture &rDepthTexture,
							bool bSet = true);

		void UnsetTextureSampler(std::string szName);
		void SetTextureSampler(std::string szName, IOESamplerState *pSampler);

		void ResetUAVs();
		void UnsetUAV(int32_t nUAV = 0, EUAVSet bSet = EUAVSet::OutputMerger);
		void SetUAVs(std::vector<IOE::Renderer::IOETexture> arrUAVs,
					 EUAVSet bSet = EUAVSet::OutputMerger);
		void SetUAV(IOE::Renderer::IOETexture &rUAV, int32_t m_nUAVIdx = 0,
					EUAVSet bSet = EUAVSet::OutputMerger, int arrOffset = 0);

	public:
		void AddViewport(float fTop, float fLeft, float fWidth, float fHeight,
						 float fMinDepth = 0.0f, float fMaxDepth = 1.0f);
		void AddViewport(IOEViewport tViewport);
		void RemoveViewport(std::int32_t nIdx);
		void ResetViewports();
		void SetupViewports();

		FORCEINLINE const IOEViewport &GetDefaultViewport() const
		{
			return m_tDefaultViewport;
		}
		FORCEINLINE std::size_t GetNumViewports() const
		{
			return m_arrViewports.size();
		}
		FORCEINLINE const IOEViewport &GetViewport(std::size_t nIdx) const
		{
			return m_arrViewports.at(nIdx);
		}

		FORCEINLINE const IOETexture &GetDefaultColourBuffer() const
		{
			return m_tDefaultColourBuffer;
		}
		FORCEINLINE IOETexture &GetDefaultColourBuffer()
		{
			return m_tDefaultColourBuffer;
		}

		FORCEINLINE const IOETexture &GetDefaultDepthStencilbuffer() const
		{
			return m_tDefaultDepthStencil;
		}
		FORCEINLINE IOETexture &GetDefaultDepthStencilbuffer()
		{
			return m_tDefaultDepthStencil;
		}

		FORCEINLINE const class IOEMaterial *GetDebugDrawMaterial() const
		{
			return m_pDebugDrawShader.get();
		}
		FORCEINLINE class IOEMaterial *GetDebugDrawMaterial()
		{
			return m_pDebugDrawShader.get();
		}

	public:
		void Resize(float fNewWidth, float fNewHeight);

		FORCEINLINE const IOE::Maths::IOECamera &GetCamera() const
		{
			return m_mCamera;
		}
		FORCEINLINE IOE::Maths::IOECamera &GetCamera()
		{
			return m_mCamera;
		}

		FORCEINLINE const IOE::Maths::IOECamera &GetDefaultCamera() const
		{
			return m_mDefaultCamera;
		}
		FORCEINLINE IOE::Maths::IOECamera &GetDefaultCamera()
		{
			return m_mDefaultCamera;
		}

		FORCEINLINE void SetCamera(IOE::Maths::IOECamera &rCamera)
		{
			m_mCamera = rCamera;
		}

		FORCEINLINE void DrawBuffer(IOEBufferBase *pVertexBuffer,
									EPrimitiveTopology eTopology,
									IOEBufferBase *pIndexBuffer = nullptr,
									std::int32_t nNumElements = -1)
		{
			m_tPlatform.DrawBuffer(pVertexBuffer, eTopology, pIndexBuffer,
								   nNumElements);
		}

		FORCEINLINE void SetPrimitiveTopology(EPrimitiveTopology eTopology)
		{
			if (m_eTopology != eTopology)
			{
				m_tPlatform.SetPrimitiveTopology(eTopology);
				m_eTopology = eTopology;
			}
		}

		FORCEINLINE EPrimitiveTopology GetPrimitiveTopology() const
		{
			return m_eTopology;
		}

	public:
		FORCEINLINE_DEBUGGABLE void
		AddDebugPoint(const IOEVector &vWorldPosition,
					  const IOEVector &vColour = Maths::ColourList::Red)
		{
			IOE_ASSERT(m_uNumDebugPoints < ms_uNumDebugPoints,
					   "Too many points! Increase static buffer size");
			if (m_uNumDebugPoints < ms_uNumDebugPoints)
			{
				m_arrDebugPointList.at(m_uNumDebugPoints++)
					.Set(vWorldPosition, vColour);
			}
		}
		FORCEINLINE_DEBUGGABLE void
		AddDebugLine(const IOEVector &vWorldStart, const IOEVector &vWorldEnd,
					 const IOEVector &vStartColour = Maths::ColourList::Red,
					 const IOEVector &vEndColour = Maths::ColourList::Red)
		{
			IOE_ASSERT(m_uNumDebugLines < ms_uNumDebugLines,
					   "Too many lines! Increase static buffer size");

			if (m_uNumDebugLines < ms_uNumDebugLines)
			{
				m_arrDebugLineList.at((m_uNumDebugLines * 2))
					.Set(vWorldStart, vStartColour);
				m_arrDebugLineList.at((m_uNumDebugLines * 2) + 1)
					.Set(vWorldEnd, vEndColour);
			}

			++m_uNumDebugLines;
		}
		FORCEINLINE_DEBUGGABLE void
		AddDebugTriangle(const IOEVector &vP0, const IOEVector &vP1,
						 const IOEVector &vP2,
						 const IOEVector &vColour0 = Maths::ColourList::Red,
						 const IOEVector &vColour1 = Maths::ColourList::Red,
						 const IOEVector &vColour2 = Maths::ColourList::Red)
		{
			IOE_ASSERT(m_uNumDebugTriangles < ms_uNumDebugTriangles,
					   "Too many triangles! Increase static buffer size");

			if (m_uNumDebugTriangles < ms_uNumDebugTriangles)
			{
				m_arrDebugTriangleList.at((m_uNumDebugTriangles * 3))
					.Set(vP0, vColour0);
				m_arrDebugTriangleList.at((m_uNumDebugTriangles * 3) + 1)
					.Set(vP1, vColour1);
				m_arrDebugTriangleList.at((m_uNumDebugTriangles * 3) + 2)
					.Set(vP2, vColour2);

				++m_uNumDebugTriangles;
			}
		}

		void AddDebugBox(const IOEVector &vCentre, const IOEVector &vExtents,
						 const IOEVector &vColour = Maths::ColourList::Red);

		FORCEINLINE_DEBUGGABLE void FlushDebugBuffers()
		{
			m_uNumDebugPoints	= 0;
			m_uNumDebugLines	 = 0;
			m_uNumDebugTriangles = 0;
		}

		FORCEINLINE const IOERasterizerState &GetRasterizerState() const
		{
			return m_tRasterizerState;
		}
		FORCEINLINE void SetRasterizerState(const IOERasterizerState &rhs)
		{
			m_tPlatform.SetRasterizerState(rhs);
		}
		FORCEINLINE void SetDepthState(const IOEDepthState &rhs,
									   std::uint32_t uStencilRef = 0)
		{
			m_tPlatform.SetDepthState(rhs, uStencilRef);
		}
		FORCEINLINE void
		SetBlendState(const IOEBlendState &rhs,
					  std::uint32_t uWriteMask = 0xFFFFFFFF,
					  const IOE::Maths::IOEVector &arrBlendFactor =
						  IOE::Maths::g_vOneVector)
		{
			m_tPlatform.SetBlendState(rhs, uWriteMask, arrBlendFactor);
		}

		FORCEINLINE const IOEDepthState &GetDefaultDepthState() const
		{
			return m_tDefaultDepthState;
		}

	public:
		std::uint32_t WriteDebugData(DebugVertex *pStartVertices)
		{
			if (m_uNumDebugPoints)
			{
				memcpy(pStartVertices, m_arrDebugPointList.data(),
					   m_uNumDebugPoints * sizeof(DebugVertex));
				pStartVertices += m_uNumDebugPoints;
			}
			if (m_uNumDebugLines)
			{
				memcpy(pStartVertices, m_arrDebugLineList.data(),
					   m_uNumDebugLines * sizeof(DebugVertex) * 2);
				pStartVertices += m_uNumDebugLines * 2;
			}
			if (m_uNumDebugTriangles)
			{
				memcpy(pStartVertices, m_arrDebugTriangleList.data(),
					   m_uNumDebugTriangles * sizeof(DebugVertex) * 3);
				pStartVertices += m_uNumDebugTriangles * 3;
			}
			// Return num bytes written to buffer
			return (m_uNumDebugPoints + m_uNumDebugLines * 2 +
					m_uNumDebugTriangles * 3) *
				sizeof(DebugVertex);
		}
		std::uint32_t GetNumDebugPoints() const
		{
			return m_uNumDebugPoints;
		}
		std::uint32_t GetNumDebugLines() const
		{
			return m_uNumDebugLines;
		}
		std::uint32_t GetNumDebugTriangles() const
		{
			return m_uNumDebugTriangles;
		}

	public:
		FORCEINLINE void SetProfileMarker(const std::string &szProfilerMark,
										  EProfileMarkerType eProfileMarker,
										  const IOE::Maths::IOEVector &vColour)
		{
			m_tPlatform.SetProfileMarker(szProfilerMark, eProfileMarker,
										 vColour);
		}

	public:
		FORCEINLINE void SetShaderIndex(EShaderType::Enum eShaderType,
										std::uint32_t uIdx)
		{
			m_arrCurrentShaderIndexes[eShaderType] = uIdx;
		}
		FORCEINLINE std::uint32_t
		GetShaderIndex(EShaderType::Enum eShaderType) const
		{
			return m_arrCurrentShaderIndexes[eShaderType];
		}

		FORCEINLINE void SetMaterialIndex(std::uint32_t uIdx)
		{
			m_uCurrentMaterialIndex = uIdx;
		}
		FORCEINLINE std::uint32_t GetMaterialIndex() const
		{
			return m_uCurrentMaterialIndex;
		}

	private:
		IOE::Maths::IOECamera m_mCamera;
		IOE::Maths::IOECamera m_mDefaultCamera;
		IOERPI_Platform m_tPlatform;

		IOEViewport m_tDefaultViewport;

		IOETexture m_tDefaultColourBuffer;
		IOETexture m_tDefaultDepthStencil;

		shared_ptr<class IOEMaterial> m_pDebugDrawShader;
		// IOEShader m_tDebugDrawShaderWidth;

		std::array<DebugVertex, ms_uNumDebugPoints> m_arrDebugPointList;
		std::uint32_t m_uNumDebugPoints;

		std::array<DebugVertex, ms_uNumDebugLines * 2> m_arrDebugLineList;
		std::uint32_t m_uNumDebugLines;

		std::array<DebugVertex, ms_uNumDebugTriangles * 3>
			m_arrDebugTriangleList;
		std::uint32_t m_uNumDebugTriangles;

		std::vector<IOEViewport> m_arrViewports;
		std::vector<std::pair<std::string, IOESamplerState *> >
			m_arrTextureSamplers;

		IOERasterizerState m_tRasterizerState;
		IOEBlendState m_tBlendState;
		IOESamplerState m_tDefaultSamplerState;
		IOEDepthState m_tDefaultDepthState;

		EPrimitiveTopology m_eTopology;

	private:
		std::uint32_t m_uCurrentMaterialIndex;
		std::uint32_t m_arrCurrentShaderIndexes[EShaderType::NumShaders];
	};

	//////////////////////////////////////////////////////////////////////////
}
}