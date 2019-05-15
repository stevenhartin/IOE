#pragma once

#include <IOE/IOERenderer/IOERPI_PlatformBase.h>

// For perf markers
#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
#include <d3d11_1.h>
#endif

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11Debug;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct ID3D11Buffer;
struct ID3D11InputLayout;

struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11SamplerState;

struct ID3DUserDefinedAnnotation;

namespace IOE
{
namespace Renderer
{
	//////////////////////////////////////////////////////////////////////////

	enum class EPrimitiveTopology;

	//////////////////////////////////////////////////////////////////////////

	template <class TCom>
	static inline void ReleaseD3DObject(TCom *&pObject)
	{
		if (pObject != nullptr)
		{
			pObject->Release();
			pObject = nullptr;
		}
	}

	extern void SetD3DObjectDebugName(ID3D11DeviceChild *pD3DObject, const char *pFmt, ...);

	//////////////////////////////////////////////////////////////////////////

	struct IOERasterizerState_Platform : public IOERasterizerState_PlatformBase
	{
		IOERasterizerState_Platform(IOERasterizerState &rState)
			: IOERasterizerState_PlatformBase(rState)
			, m_pRasterizerState(nullptr)
		{
		}

		~IOERasterizerState_Platform()
		{
			ReleaseD3DObject(m_pRasterizerState);
		}

		virtual void Setup(IOERPI *pRPI) override final;

		ID3D11RasterizerState *GetD3D11State()
		{
			return m_pRasterizerState;
		}
		const ID3D11RasterizerState *GetD3D11State() const
		{
			return m_pRasterizerState;
		}

	private:
		ID3D11RasterizerState *m_pRasterizerState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEBlendState_Platform : public IOEBlendState_PlatformBase
	{
		IOEBlendState_Platform(IOEBlendState &rState)
			: IOEBlendState_PlatformBase(rState)
			, m_pBlendState(nullptr)
		{
		}

		~IOEBlendState_Platform()
		{
			ReleaseD3DObject(m_pBlendState);
		}

		virtual void Setup(IOERPI *pRPI) override final;

		ID3D11BlendState *GetD3D11State()
		{
			return m_pBlendState;
		}
		const ID3D11BlendState *GetD3D11State() const
		{
			return m_pBlendState;
		}

	private:
		ID3D11BlendState *m_pBlendState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOESamplerState_Platform : public IOESamplerState_PlatformBase
	{
		IOESamplerState_Platform(IOESamplerState &rState)
			: IOESamplerState_PlatformBase(rState)
			, m_pSamplerState(nullptr)
		{
		}

		~IOESamplerState_Platform()
		{
			ReleaseD3DObject(m_pSamplerState);
		}

		virtual void Setup(IOERPI *pRPI) override final;

		ID3D11SamplerState *GetD3D11State()
		{
			return m_pSamplerState;
		}
		const ID3D11SamplerState *GetD3D11State() const
		{
			return m_pSamplerState;
		}

	private:
		ID3D11SamplerState *m_pSamplerState;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEDepthState_Platform : public IOEDepthState_PlatformBase
	{
		IOEDepthState_Platform(IOEDepthState &rState)
			: IOEDepthState_PlatformBase(rState)
			, m_pDepthState(nullptr)
		{
		}

		~IOEDepthState_Platform()
		{
			ReleaseD3DObject(m_pDepthState);
		}

		virtual void Setup(IOERPI *pRPI) override final;

		ID3D11DepthStencilState *GetD3D11State()
		{
			return m_pDepthState;
		}
		const ID3D11DepthStencilState *GetD3D11State() const
		{
			return m_pDepthState;
		}

	private:
		ID3D11DepthStencilState *m_pDepthState;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOERPI_Platform : public IOERPI_PlatformBase
	{
		static const int32_t ms_nNumRenderTargets = 8;
		static const int32_t ms_nNumUAVs		  = 8;

	public:
		IOERPI_Platform();

		virtual void SetupViewports() override final;
		virtual void ClearDepth(IOE::Maths::IOEVector &vColour) override final;
		virtual void Clear(IOE::Maths::IOEVector &vColour) override final;
		virtual void ClearUAV(IOETexture *pUAV,
							  IOE::Maths::IOEVector &vColour = Maths::ColourList::Black) override final;
		virtual void ClearUAV(IOETexture *pUAV, std::uint32_t uARGB) override final;
		virtual void ResetUAVs();
		virtual void ClearBuffer(IOETexture *pBuffer, const void *pSrcValue, std::size_t uSrcByteSize) override final;

		virtual void StartRender() override final;
		virtual void FinishRender() override final;

		virtual void SetRasterizerState(const IOERasterizerState &rhs) override final;
		virtual void
		SetBlendState(const IOEBlendState &rhs, std::uint32_t uWriteMask = 0xFFFFFFFF,
					  const IOE::Maths::IOEVector &srrBlendFactor = IOE::Maths::g_vOneVector) override final;
		virtual void SetSamplerState(const IOESamplerState &rhs) override final;
		virtual void SetDepthState(const IOEDepthState &rhs, std::uint32_t uStencilRef = 0) override final;

		virtual void SetProfileMarker(const std::string &szProfilerMark, EProfileMarkerType eProfileMarker,
									  const IOE::Maths::IOEVector &vColour) override final;

	public:
		virtual void OnInit(weak_ptr<IOERPI> pRPI) override final;
		virtual void Resize(float fNewWidth, float fNewHeight) override final;

	public:
		FORCEINLINE IDXGISwapChain *GetSwapChain()
		{
			return m_pDXGISwapChain.get();
		}
		FORCEINLINE const IDXGISwapChain *GetSwapChain() const
		{
			return m_pDXGISwapChain.get();
		}

		FORCEINLINE ID3D11Device *GetDevice()
		{
			return m_pD3DDevice.get();
		}
		FORCEINLINE const ID3D11Device *GetDevice() const
		{
			return m_pD3DDevice.get();
		}

		FORCEINLINE ID3D11Debug *GetDebugDevice()
		{
			return m_pD3DDebug.get();
		}
		FORCEINLINE const ID3D11Debug *GetDebugDevice() const
		{
			return m_pD3DDebug.get();
		}

		FORCEINLINE ID3D11DeviceContext *GetDeviceContext()
		{
			return m_pD3DDeviceContext.get();
		}
		FORCEINLINE const ID3D11DeviceContext *GetDeviceContext() const
		{
			return m_pD3DDeviceContext.get();
		}

	public:
		virtual void ResetState() override final;

		virtual void UnsetRenderTarget(int32_t nRenderTarget = 0) override final;
		virtual void SetRenderTarget(IOE::Renderer::IOETexture &rTexture, int32_t nRenderTargetIdx = 0,
									 bool bSet = true) override final;
		virtual void SetRenderTargets(std::vector<IOE::Renderer::IOETexture> arrTextures,
									  bool bSet = true) override final;

		virtual int32_t GetNumRenderTargets() const;

		virtual void SetDepthTarget(IOE::Renderer::IOETexture &rDepthTexture, bool bSet = true) override final;

	public:
		virtual void UnsetUAV(int32_t nUAV, EUAVSet bSet) override final;
		virtual void SetUAV(IOE::Renderer::IOETexture &rTexture, int32_t nUAVIdx, EUAVSet bSet,
							int arrOffset = 0) override final;
		virtual void SetUAVs(std::vector<IOE::Renderer::IOETexture> arrTextures, EUAVSet bSet) override final;

		virtual int32_t GetNumUAVs(EUAVSet bSet) const;
		int32_t GetFirstUAVSlotIndex(EUAVSet bSet) const;
		int32_t GetMaxUAVIndex(EUAVSet bSet) const;

	public:
		virtual void DrawBuffer(IOEBufferBase *pVertexBuffer, EPrimitiveTopology eTopology,
								IOEBufferBase *pIndexBuffer = nullptr, std::int32_t nNumElements = -1) override final;

		virtual void SetPrimitiveTopology(EPrimitiveTopology eTopology) override final;

	private:
		void SetupD3D();
		void ReleaseD3D();
		void CreateDebugBuffers();

		void RefreshRenderTargetsAndUAVs(EUAVSet bSet);
		int32_t GetUAVBuffer(std::array<ID3D11UnorderedAccessView *, ms_nNumUAVs> &rarrUAVs, EUAVSet bSet);

	private:
		TComSharedPtr<IDXGISwapChain> m_pDXGISwapChain;
		TComSharedPtr<ID3D11Device> m_pD3DDevice;
		TComSharedPtr<ID3D11Debug> m_pD3DDebug;
		TComSharedPtr<ID3D11DeviceContext> m_pD3DDeviceContext;

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
		TComSharedPtr<ID3DUserDefinedAnnotation> m_pPerfAnnotations;
#endif

		TComSharedPtr<ID3D11RenderTargetView> m_pD3DDefaultRenderTargetView;
		TComSharedPtr<ID3D11RenderTargetView> m_pD3DRenderTargetView;
		TComSharedPtr<ID3D11DepthStencilView> m_pD3DDefaultDepthStencilView;
		TComSharedPtr<ID3D11DepthStencilView> m_pD3DDepthStencilView;

		TComSharedPtr<ID3D11Texture2D> m_pD3DDepthStencilBuffer;

		TComSharedPtr<ID3D11DepthStencilState> m_pDefaultDepthStencilState;
		TComSharedPtr<ID3D11SamplerState> m_pDefaultSamplerState;
		// TComSharedPtr<ID3D11BlendState> m_pDefaultBlendState;

		TComSharedPtr<ID3D11Buffer> m_pVertexBuffer;
		TComSharedPtr<ID3D11InputLayout> m_pDebugInputLayout;

		std::array<TComSharedPtr<ID3D11RenderTargetView>, ms_nNumRenderTargets> m_arrRenderTargets;
		int32_t m_nNumRenderTargets;

		std::array<std::pair<TComSharedPtr<ID3D11UnorderedAccessView>, int32_t>, ms_nNumUAVs> m_arrCSUAVs;
		int32_t m_nNumCSUAVs;
		std::array<std::pair<TComSharedPtr<ID3D11UnorderedAccessView>, int32_t>, ms_nNumUAVs> m_arrUAVs;
		uint32_t m_arrUAVsWriteOffsets[ms_nNumUAVs];
		int32_t m_nNumUAVs;
	};

	//////////////////////////////////////////////////////////////////////////
}
}