#include <memory>

#include <d3d11.h>

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOECore/IOEHeaders.h>
#include <IOE/IOECore/IOEManagerContainer.h>

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
#include <d3d11_1.h>
#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")

#endif // (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)

#include <IOE/IOEExceptions/IOEException_Enum.h>
#include <IOE/IOEExceptions/IOEException_Renderer.h>

#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOEApplication/IOEApplication_PlatformBase.h>
#include EXTERNAL_PLATFORM_INCLUDE(IOEApplication, IOEApplication_Platform.h)

#include <IOE/IOEInput/IOEInputKeyboard.h>
#include <IOE/IOEInput/IOEInputKeys.h>
#include <IOE/IOEInput/IOEInputManager.h>
#include <IOE/IOEInput/IOEInputMouse.h>

#include <IOE/IOEMaths/AABB.h>
#include <IOE/IOEMaths/IOERay.h>

#include "../IOEBuffer.h"
#include "../IOEMaterial.h"
#include "../IOEModel.h"
#include "../IOEModelManager.h"
#include "../IOERPI.h"
#include "../IOEShader.h"
#include "../IOEVertexFormat.h"
#include "../IOEViewport.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const D3D11_INPUT_ELEMENT_DESC g_arrDebugVertexDescription[] = {
	{
		"POSITION",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		offsetof(IOE::Renderer::DebugVertex, vPos),
		D3D11_INPUT_PER_VERTEX_DATA,
		0,
	},
	{
		"COLOR",
		0,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		0,
		offsetof(IOE::Renderer::DebugVertex, vColour),
		D3D11_INPUT_PER_VERTEX_DATA,
		0,
	},
};

UINT g_uNumVertexSemantics = sizeof(g_arrDebugVertexDescription) / sizeof(g_arrDebugVertexDescription[0]);

//////////////////////////////////////////////////////////////////////////

#define BEGIN_NAMES(TYPE)                                                                                             \
	const char *GetName##TYPE(int v)                                                                                  \
	{                                                                                                                 \
		switch (v)                                                                                                    \
		{                                                                                                             \
		default:                                                                                                      \
			return "?" #TYPE "?";

#define END_NAMES()                                                                                                   \
	}                                                                                                                 \
	}

#define NAME(X)                                                                                                       \
	case (X):                                                                                                         \
		return #X;

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_FEATURE_LEVEL)
NAME(D3D_FEATURE_LEVEL_9_1)
NAME(D3D_FEATURE_LEVEL_9_2)
NAME(D3D_FEATURE_LEVEL_9_3)
NAME(D3D_FEATURE_LEVEL_10_0)
NAME(D3D_FEATURE_LEVEL_10_1)
NAME(D3D_FEATURE_LEVEL_11_0)
END_NAMES()

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_VARIABLE_CLASS)
NAME(D3D_SVC_SCALAR)
NAME(D3D_SVC_VECTOR)
NAME(D3D_SVC_MATRIX_ROWS)
NAME(D3D_SVC_MATRIX_COLUMNS)
NAME(D3D_SVC_OBJECT)
NAME(D3D_SVC_STRUCT)
NAME(D3D_SVC_INTERFACE_CLASS)
NAME(D3D_SVC_INTERFACE_POINTER)
END_NAMES()

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_VARIABLE_TYPE)
NAME(D3D_SVT_VOID)
NAME(D3D_SVT_BOOL)
NAME(D3D_SVT_INT)
NAME(D3D_SVT_FLOAT)
NAME(D3D_SVT_STRING)
NAME(D3D_SVT_TEXTURE)
NAME(D3D_SVT_TEXTURE1D)
NAME(D3D_SVT_TEXTURE2D)
NAME(D3D_SVT_TEXTURE3D)
NAME(D3D_SVT_TEXTURECUBE)
NAME(D3D_SVT_SAMPLER)
NAME(D3D_SVT_SAMPLER1D)
NAME(D3D_SVT_SAMPLER2D)
NAME(D3D_SVT_SAMPLER3D)
NAME(D3D_SVT_SAMPLERCUBE)
NAME(D3D_SVT_PIXELSHADER)
NAME(D3D_SVT_VERTEXSHADER)
NAME(D3D_SVT_PIXELFRAGMENT)
NAME(D3D_SVT_VERTEXFRAGMENT)
NAME(D3D_SVT_UINT)
NAME(D3D_SVT_UINT8)
NAME(D3D_SVT_GEOMETRYSHADER)
NAME(D3D_SVT_RASTERIZER)
NAME(D3D_SVT_DEPTHSTENCIL)
NAME(D3D_SVT_BLEND)
NAME(D3D_SVT_BUFFER)
NAME(D3D_SVT_CBUFFER)
NAME(D3D_SVT_TBUFFER)
NAME(D3D_SVT_TEXTURE1DARRAY)
NAME(D3D_SVT_TEXTURE2DARRAY)
NAME(D3D_SVT_RENDERTARGETVIEW)
NAME(D3D_SVT_DEPTHSTENCILVIEW)
NAME(D3D_SVT_TEXTURE2DMS)
NAME(D3D_SVT_TEXTURE2DMSARRAY)
NAME(D3D_SVT_TEXTURECUBEARRAY)
NAME(D3D_SVT_HULLSHADER)
NAME(D3D_SVT_DOMAINSHADER)
NAME(D3D_SVT_INTERFACE_POINTER)
NAME(D3D_SVT_COMPUTESHADER)
NAME(D3D_SVT_DOUBLE)
NAME(D3D_SVT_RWTEXTURE1D)
NAME(D3D_SVT_RWTEXTURE1DARRAY)
NAME(D3D_SVT_RWTEXTURE2D)
NAME(D3D_SVT_RWTEXTURE2DARRAY)
NAME(D3D_SVT_RWTEXTURE3D)
NAME(D3D_SVT_RWBUFFER)
NAME(D3D_SVT_BYTEADDRESS_BUFFER)
NAME(D3D_SVT_RWBYTEADDRESS_BUFFER)
NAME(D3D_SVT_STRUCTURED_BUFFER)
NAME(D3D_SVT_RWSTRUCTURED_BUFFER)
NAME(D3D_SVT_APPEND_STRUCTURED_BUFFER)
NAME(D3D_SVT_CONSUME_STRUCTURED_BUFFER)
END_NAMES()

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SHADER_INPUT_TYPE)
NAME(D3D_SIT_CBUFFER)
NAME(D3D_SIT_TBUFFER)
NAME(D3D_SIT_TEXTURE)
NAME(D3D_SIT_SAMPLER)
NAME(D3D_SIT_UAV_RWTYPED)
NAME(D3D_SIT_STRUCTURED)
NAME(D3D_SIT_UAV_RWSTRUCTURED)
NAME(D3D_SIT_BYTEADDRESS)
NAME(D3D_SIT_UAV_RWBYTEADDRESS)
NAME(D3D_SIT_UAV_APPEND_STRUCTURED)
NAME(D3D_SIT_UAV_CONSUME_STRUCTURED)
NAME(D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
END_NAMES()

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_RESOURCE_RETURN_TYPE)
NAME(D3D_RETURN_TYPE_UNORM)
NAME(D3D_RETURN_TYPE_SNORM)
NAME(D3D_RETURN_TYPE_SINT)
NAME(D3D_RETURN_TYPE_UINT)
NAME(D3D_RETURN_TYPE_FLOAT)
NAME(D3D_RETURN_TYPE_MIXED)
NAME(D3D_RETURN_TYPE_DOUBLE)
NAME(D3D_RETURN_TYPE_CONTINUED)
END_NAMES()

//////////////////////////////////////////////////////////////////////

BEGIN_NAMES(D3D_SRV_DIMENSION)
NAME(D3D_SRV_DIMENSION_UNKNOWN)
NAME(D3D_SRV_DIMENSION_BUFFER)
NAME(D3D_SRV_DIMENSION_TEXTURE1D)
NAME(D3D_SRV_DIMENSION_TEXTURE1DARRAY)
NAME(D3D_SRV_DIMENSION_TEXTURE2D)
NAME(D3D_SRV_DIMENSION_TEXTURE2DARRAY)
NAME(D3D_SRV_DIMENSION_TEXTURE2DMS)
NAME(D3D_SRV_DIMENSION_TEXTURE2DMSARRAY)
NAME(D3D_SRV_DIMENSION_TEXTURE3D)
NAME(D3D_SRV_DIMENSION_TEXTURECUBE)
NAME(D3D_SRV_DIMENSION_TEXTURECUBEARRAY)
NAME(D3D_SRV_DIMENSION_BUFFEREX)
END_NAMES()

//////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	void SetD3DObjectDebugName(ID3D11DeviceChild *pD3DObject, const char *pFmt, ...)
	{
		va_list v;
		char buf[128];

		va_start(v, pFmt);

		_vsnprintf_s<128>(buf, sizeof buf, pFmt, v);
		buf[sizeof buf - 1] = 0;

		va_end(v);

		pD3DObject->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(strlen(buf)), buf);
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_PRIMITIVE_TOPOLOGY
	GetPrimitiveFromEPrimitive(EPrimitiveTopology eType)
	{
		switch (eType)
		{
		case EPrimitiveTopology::Point:
			return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case EPrimitiveTopology::LineList:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case EPrimitiveTopology::LineStrip:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case EPrimitiveTopology::LineListWithAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
		case EPrimitiveTopology::LineStripWithAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
		case EPrimitiveTopology::TriangleList:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case EPrimitiveTopology::TriangleStrip:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case EPrimitiveTopology::TriangleListWithAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
		case EPrimitiveTopology::TriangleStripWithAdjacency:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
		default:
			return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	IOERPI_Platform::IOERPI_Platform()
		: m_nNumRenderTargets(0)
		, m_nNumUAVs(0)
		, m_nNumCSUAVs(0)
	{
		for (int32_t nIdx(0); nIdx < ms_nNumUAVs; ++nIdx)
		{
			m_arrUAVsWriteOffsets[nIdx] = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::OnInit(weak_ptr<IOERPI> pRPI)
	{
		IOERPI_PlatformBase::OnInit(pRPI);

		SetupD3D();
		CreateDebugBuffers();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::Resize(float fNewWidth, float fNewHeight)
	{
		if (m_pDXGISwapChain == nullptr)
		{
			return;
		}

		if (nullptr != m_pD3DDebug)
		{
			m_pD3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
		}

		// Even though this has been resized, D3D can't resize swap chain
		// unless we have
		// cleaned up all references to back buffers first.
		IOERPI *pRPI(GetRPI());
		pRPI->GetDefaultColourBuffer().GetPlatform().Cleanup();
		pRPI->GetDefaultDepthStencilbuffer().GetPlatform().Cleanup();

		m_pD3DDeviceContext->ClearState();
		m_pD3DDeviceContext->Flush();

		// Get swap chain desc.
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		if (FAILED(m_pDXGISwapChain->GetDesc(&swapChainDesc)))
		{
			return;
		}

		// Resize swap chain buffers.
		if (FAILED(m_pDXGISwapChain->ResizeBuffers(swapChainDesc.BufferCount, static_cast<UINT>(fNewWidth),
												   static_cast<UINT>(fNewHeight), swapChainDesc.BufferDesc.Format,
												   swapChainDesc.Flags)))
		{
			if (nullptr != m_pD3DDebug)
			{
				m_pD3DDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
			}
			return;
		}

		// Resetup the back buffers
		pRPI->GetDefaultColourBuffer().GetPlatform().Setup();
		m_pD3DDefaultRenderTargetView =
			TComPtrCast<ID3D11RenderTargetView>(pRPI->GetDefaultColourBuffer().GetPlatform().GetViewShared());

		pRPI->GetDefaultDepthStencilbuffer().GetPlatform().Cleanup();
		pRPI->GetDefaultDepthStencilbuffer().GetPlatform().Setup();
		m_pD3DDefaultDepthStencilView =
			TComPtrCast<ID3D11DepthStencilView>(pRPI->GetDefaultDepthStencilbuffer().GetPlatform().GetViewShared());

		m_pD3DRenderTargetView = m_pD3DDefaultRenderTargetView;
		m_pD3DDepthStencilView = m_pD3DDefaultDepthStencilView;

		SetD3DObjectDebugName(m_pD3DDefaultRenderTargetView.get(), "DefaultRenderTargetView");
		SetD3DObjectDebugName(m_pD3DDefaultDepthStencilView.get(), "DefaultDepthStencilView");

		SetupViewports();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::Clear(IOE::Maths::IOEVector &vColour)
	{
		float arrColour[4];
		vColour.Get(arrColour[0], arrColour[1], arrColour[2], arrColour[3]);
		for (int32_t nIdx(0); nIdx < m_nNumRenderTargets; ++nIdx)
		{
			m_pD3DDeviceContext->ClearRenderTargetView(m_arrRenderTargets[nIdx].get(), arrColour);
		}
		m_pD3DDeviceContext->ClearDepthStencilView(m_pD3DDepthStencilView.get(),
												   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ClearDepth(IOE::Maths::IOEVector &vColour)
	{
		float arrColour[4];
		vColour.Get(arrColour[0], arrColour[1], arrColour[2], arrColour[3]);
		m_pD3DDeviceContext->ClearDepthStencilView(m_pD3DDepthStencilView.get(),
												   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ResetUAVs()
	{
		for (int nIdx = 0; nIdx < m_nNumUAVs; ++nIdx)
		{
			m_arrUAVs[nIdx].first  = nullptr;
			m_arrUAVs[nIdx].second = -1;
		}
		m_nNumUAVs = 0;

		for (int nIdx = 0; nIdx < m_nNumCSUAVs; ++nIdx)
		{
			m_arrCSUAVs[nIdx].first  = nullptr;
			m_arrCSUAVs[nIdx].second = -1;
		}

		m_nNumCSUAVs = 0;

		RefreshRenderTargetsAndUAVs(EUAVSet::ComputeShader);
		RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ClearUAV(IOETexture *pUAV, IOE::Maths::IOEVector &vColour /*= Maths::ColourList::Black*/)
	{
		IOE_ASSERT(pUAV->GetBufferDescription() & EBufferDescription::UnorderedAccess, "Incompatable texture type");
		float arrColour[4];
		vColour.Get(arrColour[0], arrColour[1], arrColour[2], arrColour[3]);
		m_pD3DDeviceContext->ClearUnorderedAccessViewFloat(
			static_cast<ID3D11UnorderedAccessView *>(pUAV->GetPlatform().GetView()), arrColour);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ClearUAV(IOETexture *pUAV, std::uint32_t uARGB)
	{
		IOE_ASSERT(pUAV->GetBufferDescription() & EBufferDescription::UnorderedAccess, "Incompatable texture type");
		if (pUAV->CanAllowRawValues())
		{
			std::uint32_t arrColour[] = { uARGB, uARGB, uARGB, uARGB };
			m_pD3DDeviceContext->ClearUnorderedAccessViewUint(
				static_cast<ID3D11UnorderedAccessView *>(pUAV->GetPlatform().GetView()), arrColour);
		}
		else
		{
			std::uint32_t arrColour[4] = { (uARGB >> 24) & 0xFF, (uARGB >> 16) & 0xFF, (uARGB >> 8) & 0xFF,
										   (uARGB >> 0) & 0xFF };
			m_pD3DDeviceContext->ClearUnorderedAccessViewUint(
				static_cast<ID3D11UnorderedAccessView *>(pUAV->GetPlatform().GetView()), arrColour);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ClearBuffer(IOETexture *pBuffer, const void *pSrcValue, std::size_t uSrcByteSize)
	{
		IOE_ASSERT(pBuffer->GetTextureType() == ETextureType::GenericBuffer &&
					   pBuffer->GetTextureFormat() == ETextureFormat::Unknown,
				   "Invalid texture type");

		const std::uint8_t *pTypedSrcValue = reinterpret_cast<const std::uint8_t *>(pSrcValue);

		std::size_t nNumElements	 = static_cast<std::size_t>(pBuffer->GetArrayLength());
		std::size_t nElementSize	 = static_cast<std::size_t>(pBuffer->GetWidth());
		std::size_t uNumBytesToWrite = nNumElements * nElementSize;

		auto arrData = std::make_unique<std::vector<std::uint8_t> >(uNumBytesToWrite);
		for (std::size_t nByteIdx(0); nByteIdx < uNumBytesToWrite; ++nByteIdx)
		{
			arrData->at(nByteIdx) = pTypedSrcValue[nByteIdx % uSrcByteSize];
		}

		GetDeviceContext()->UpdateSubresource(pBuffer->GetPlatform().GetD3D11Texture(), 0, nullptr, arrData->data(),
											  (UINT)nElementSize, (UINT)nNumElements);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetupD3D()
	{
		IOE::Application::IOEApplication_Platform &rPlatform(IOE::Core::g_pEngine->GetApplication()->GetPlatform());

		UINT uFlags = 0;
#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_SHADER_DEBUG == IOE_ON)
		std::cout << "Turning debug device on!" << std::endl;
		uFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // IOE_DEBUG

		DXGI_SWAP_CHAIN_DESC tSwapChainDesc;

		// 32-bit RGBA frame buffer, initially the same size as the window's
		// client area.
		tSwapChainDesc.BufferDesc.Width					  = 0;
		tSwapChainDesc.BufferDesc.Height				  = 0;
		tSwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
		tSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		tSwapChainDesc.BufferDesc.Format				  = DXGI_FORMAT_B8G8R8A8_UNORM;
		tSwapChainDesc.BufferDesc.ScanlineOrdering		  = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		tSwapChainDesc.BufferDesc.Scaling				  = DXGI_MODE_SCALING_UNSPECIFIED;

		// No antialiasing.
		tSwapChainDesc.SampleDesc.Count   = 1;
		tSwapChainDesc.SampleDesc.Quality = 0;

		tSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		tSwapChainDesc.BufferCount = 2;

		tSwapChainDesc.OutputWindow = rPlatform.GetApplicationWindow();
#if (defined(IOE_FINAL))
		tSwapChainDesc.Windowed = TRUE;
#else
		tSwapChainDesc.Windowed = TRUE;
#endif

		tSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

		tSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		D3D_FEATURE_LEVEL tD3DFeatureLevel;

		D3D_DRIVER_TYPE tDriverType;
		if (IsInSoftwareMode())
		{
			tDriverType = D3D_DRIVER_TYPE_REFERENCE;
		}
		else
		{
			tDriverType = D3D_DRIVER_TYPE_HARDWARE;
		}

		IDXGIFactory *pIDXGIFactory;
		CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);

		UINT i = 0;
		IDXGIAdapter *pAdapter;
		std::vector<IDXGIAdapter *> vAdapters;
		while (pIDXGIFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
		{
			vAdapters.push_back(pAdapter);
			++i;
		}

		IDXGIAdapter *pBestDevice(NULL);
		SIZE_T uBestVideoMemory(0);
		if (vAdapters.size() == 1)
		{
			DXGI_ADAPTER_DESC tDesc;
			vAdapters[0]->GetDesc(&tDesc);

			std::cout << "Default Device: "
					  << IOE::Core::Algorithm::ConvertWideToNarrow(std::wstring(tDesc.Description)).c_str()
					  << std::endl;
		}
		if (vAdapters.size() > 1)
		{
			for (IDXGIAdapter *pAdapter : vAdapters)
			{
				DXGI_ADAPTER_DESC tDesc;
				pAdapter->GetDesc(&tDesc);

				std::cout << "Found Device: "
						  << IOE::Core::Algorithm::ConvertWideToNarrow(std::wstring(tDesc.Description)).c_str()
						  << std::endl;

				if (tDesc.DedicatedVideoMemory > uBestVideoMemory)
				{
					pBestDevice		 = pAdapter;
					uBestVideoMemory = tDesc.DedicatedVideoMemory;
				}
			}
			DXGI_ADAPTER_DESC tDesc;
			pBestDevice->GetDesc(&tDesc);

			std::cout << "Chosen Device: "
					  << IOE::Core::Algorithm::ConvertWideToNarrow(std::wstring(tDesc.Description)).c_str()
					  << std::endl;
		}

		const D3D_FEATURE_LEVEL arrRequestedLevels[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};

		IDXGISwapChain *pSwapChain;
		ID3D11Device *pDevice;
		ID3D11DeviceContext *pDeviceContext;
		if (FAILED(D3D11CreateDeviceAndSwapChain(
				pBestDevice, vAdapters.size() > 1 ? D3D_DRIVER_TYPE_UNKNOWN : tDriverType, NULL, uFlags,
				arrRequestedLevels, _countof(arrRequestedLevels), D3D11_SDK_VERSION, &tSwapChainDesc, &pSwapChain,
				&pDevice, &tD3DFeatureLevel, &pDeviceContext)))
		{
			ReleaseD3D();
			THROW_IOE_RENDERER_EXCEPTION("Failed to create device and swap chain!", E_ERROR_FAILED_TO_CREATE_DEVICE);
		}
		m_pDXGISwapChain	= pSwapChain;
		m_pD3DDevice		= pDevice;
		m_pD3DDeviceContext = pDeviceContext;

		m_pD3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void **)&m_pD3DDebug);

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
		m_pD3DDevice->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void **)&m_pPerfAnnotations);
#endif

		if (m_pD3DDevice != nullptr)
		{
			ID3D11InfoQueue *d3dInfoQueue = nullptr;
			if (SUCCEEDED(m_pD3DDevice->QueryInterface(__uuidof(ID3D11InfoQueue), (void **)&d3dInfoQueue)))
			{
				D3D11_MESSAGE_ID hide[] = { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
											// Add more message IDs here as needed
											D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET };

				D3D11_INFO_QUEUE_FILTER filter;
				memset(&filter, 0, sizeof(filter));
				filter.DenyList.NumIDs  = _countof(hide);
				filter.DenyList.pIDList = hide;
				d3dInfoQueue->AddStorageFilterEntries(&filter);
				d3dInfoQueue->Release();
			}
		}
		GetRPI()->Resize(static_cast<float>(rPlatform.GetWidth()), static_cast<float>(rPlatform.GetHeight()));

		std::cout << "Feature level: " << GetNameD3D_FEATURE_LEVEL(tD3DFeatureLevel) << std::endl;
		;
		std::cout << "Got D3D11Debug: " << (m_pD3DDebug ? "YES" : "NO") << std::endl;

		if (tD3DFeatureLevel < D3D_FEATURE_LEVEL_11_0)
		{
			THROW_IOE_RENDERER_EXCEPTION_VARGS(
				"Expected feature level %s or above, received %s", E_ERROR_INVALID_HARDWARE,
				GetNameD3D_FEATURE_LEVEL(D3D_FEATURE_LEVEL_11_0), GetNameD3D_FEATURE_LEVEL(tD3DFeatureLevel));
		}

		// 		D3D11_BLEND_DESC tBlendDesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
		// 		m_pD3DDevice->CreateBlendState(&tBlendDesc,
		// &m_pDefaultBlendState);

		D3D11_SAMPLER_DESC tSamplerDesc = CD3D11_SAMPLER_DESC(D3D11_DEFAULT);
		tSamplerDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
		tSamplerDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
		tSamplerDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
		// 		tSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		// 		tSamplerDesc.MaxAnisotropy = 16;
		tSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		ID3D11SamplerState *pSampler;
		m_pD3DDevice->CreateSamplerState(&tSamplerDesc, &pSampler);
		m_pDefaultSamplerState = pSampler;

		D3D11_DEPTH_STENCIL_DESC tDepthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
		tDepthStencilDesc.DepthEnable			   = TRUE;
		tDepthStencilDesc.DepthWriteMask		   = D3D11_DEPTH_WRITE_MASK_ALL;
		tDepthStencilDesc.DepthFunc				   = D3D11_COMPARISON_LESS_EQUAL;
		ID3D11DepthStencilState *pDepthStencilState;
		m_pD3DDevice->CreateDepthStencilState(&tDepthStencilDesc, &pDepthStencilState);
		m_pDefaultDepthStencilState = pDepthStencilState;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ReleaseD3D()
	{
		// this->ReleaseRenderTargetsAndViews();
		m_pDXGISwapChain->SetFullscreenState(FALSE, NULL);

		m_pDXGISwapChain.reset();
		m_pD3DDebug.reset();
		m_pD3DDevice.reset();
		m_pD3DDeviceContext.reset();

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
		m_pPerfAnnotations.reset();
#endif

		m_pDefaultDepthStencilState.reset();
		// m_pDefaultBlendState.reset();
		m_pDefaultSamplerState.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetupViewports()
	{
		std::vector<D3D11_VIEWPORT> arrViewports;

		IOERPI *pRPI(GetRPI());
		arrViewports.resize(pRPI->GetNumViewports());

		for (std::int32_t nIdx(0); nIdx < pRPI->GetNumViewports(); ++nIdx)
		{
			arrViewports[nIdx] = reinterpret_cast<const D3D11_VIEWPORT &>(pRPI->GetViewport(nIdx));
		}

		m_pD3DDeviceContext->RSSetViewports(static_cast<UINT>(arrViewports.size()), arrViewports.data());
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::FinishRender()
	{
		// Render the debug buffers

#if 1
		// Map and update the debug vertex buffer
		D3D11_MAPPED_SUBRESOURCE tMappedBuffer;
		if (!SUCCEEDED(m_pD3DDeviceContext->Map(m_pVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &tMappedBuffer)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Unable to map debug vertex buffer", E_ERROR_RENDERER_UNKNOWN);
		}
		DebugVertex *pVertices(static_cast<DebugVertex *>(tMappedBuffer.pData));
		bool bRenderDebug(IOERPI::GetSingletonPtr()->WriteDebugData(pVertices) > 0);
		m_pD3DDeviceContext->Unmap(m_pVertexBuffer.get(), 0);
		if (bRenderDebug)
		{
			IOERPI *pRPI(GetRPI());
			pRPI->GetDebugDrawMaterial()->Set(pRPI);

			uint32_t uRendered(0);
			UINT uStride(static_cast<UINT>(sizeof(DebugVertex)));
			ID3D11Buffer *pBuffer = m_pVertexBuffer.get();
			m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &uStride, &uRendered);

			std::uint32_t uNumPoints(pRPI->GetNumDebugPoints());
			std::uint32_t uNumLines(pRPI->GetNumDebugLines());
			std::uint32_t uNumTris(pRPI->GetNumDebugTriangles());

			if (uNumPoints)
			{
				pRPI->SetPrimitiveTopology(EPrimitiveTopology::Point);
				m_pD3DDeviceContext->Draw(uNumPoints, 0);
			}
			if (uNumLines)
			{
				pRPI->SetPrimitiveTopology(EPrimitiveTopology::LineList);
				m_pD3DDeviceContext->Draw(uNumLines * 2, uNumPoints);
			}
			if (uNumTris)
			{
				pRPI->SetPrimitiveTopology(EPrimitiveTopology::TriangleList);
				m_pD3DDeviceContext->Draw(uNumTris * 3, uNumPoints + uNumLines * 2);
			}
		}
#endif

		m_pDXGISwapChain->Present(0, 0);

		m_nNumRenderTargets = 0;
		for (int32_t nIdx(0); nIdx < ms_nNumRenderTargets; ++nIdx)
		{
			m_arrRenderTargets[nIdx] = nullptr;
		}
		RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::StartRender()
	{
		IOERPI *pRPI(GetRPI());

		pRPI->SetCamera(pRPI->GetDefaultCamera());

		m_nNumRenderTargets = 1;
		m_arrRenderTargets[0] =
			TComPtrCast<ID3D11RenderTargetView>(pRPI->GetDefaultColourBuffer().GetPlatform().GetViewShared());
		RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);

		pRPI->SetRasterizerState(pRPI->GetRasterizerState());

		pRPI->ResetViewports();
		SetupViewports();

		pRPI->GetCamera().UpdateViewMatrices();

		m_pD3DDeviceContext->VSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
		m_pD3DDeviceContext->PSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
		m_pD3DDeviceContext->CSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
		m_pD3DDeviceContext->GSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
		m_pD3DDeviceContext->HSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
		m_pD3DDeviceContext->DSSetSamplers(0, 1, &m_pDefaultSamplerState.p);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::CreateDebugBuffers()
	{
		D3D11_BUFFER_DESC tBufferDesc;
		tBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		tBufferDesc.ByteWidth = sizeof(DebugVertex) * IOERPI::ms_uNumDebugPoints +
			sizeof(DebugVertex) * IOERPI::ms_uNumDebugLines * 2 +
			sizeof(DebugVertex) * IOERPI::ms_uNumDebugTriangles * 3;
		tBufferDesc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
		tBufferDesc.MiscFlags			= 0;
		tBufferDesc.Usage				= D3D11_USAGE_DYNAMIC;
		tBufferDesc.StructureByteStride = 0;

		if (!SUCCEEDED(m_pD3DDevice->CreateBuffer(&tBufferDesc, nullptr, &m_pVertexBuffer)))
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create debug buffers", E_ERROR_RENDERER_UNKNOWN);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::UnsetRenderTarget(int32_t nRenderTarget /*=0*/)
	{
		if (m_arrRenderTargets[nRenderTarget] != nullptr)
		{
			m_arrRenderTargets[nRenderTarget] = nullptr;
			--m_nNumRenderTargets;
		}
		RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetRenderTarget(IOE::Renderer::IOETexture &rTexture, int32_t nRenderTargetIdx /*=0*/,
										  bool bSet /*=true*/)
	{
		if (m_arrRenderTargets[nRenderTargetIdx] == nullptr)
		{
			++m_nNumRenderTargets;
		}
		m_arrRenderTargets[nRenderTargetIdx] =
			TComPtrCast<ID3D11RenderTargetView>(rTexture.GetPlatform().GetViewShared());
		if (bSet)
		{
			RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetRenderTargets(std::vector<IOE::Renderer::IOETexture> arrTextures, bool bSet /*=true*/)
	{
		m_nNumRenderTargets = 0;
		for (auto &rTexture : arrTextures)
		{
			m_arrRenderTargets[m_nNumRenderTargets++] =
				TComPtrCast<ID3D11RenderTargetView>(rTexture.GetPlatform().GetViewShared());
		}
		if (bSet)
		{
			RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	int32_t IOERPI_Platform::GetNumRenderTargets() const
	{
		if (m_nNumRenderTargets == 0)
		{
			if (m_arrRenderTargets[0] == nullptr)
			{
				return 0;
			}
			return 1;
		}
		return m_nNumRenderTargets;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetDepthTarget(IOE::Renderer::IOETexture &rDepthTexture, bool bSet /*=true*/)
	{
		m_pD3DDepthStencilView = TComPtrCast<ID3D11DepthStencilView>(rDepthTexture.GetPlatform().GetViewShared());
		if (bSet)
		{
			RefreshRenderTargetsAndUAVs(EUAVSet::OutputMerger);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::UnsetUAV(int32_t nUAV, EUAVSet bSet)
	{
		int32_t &rNumUAVs = (bSet == EUAVSet::ComputeShader) ? m_nNumCSUAVs : m_nNumUAVs;
		auto &rarrUAVs	= bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;

		if (bSet != EUAVSet::ComputeShader)
		{
			// nUAV += GetNumRenderTargets();
		}
		int32_t nFoundIdx = -1;
		for (int nUavIdx = 0; nUavIdx < rNumUAVs; ++nUavIdx)
		{
			if (rarrUAVs[nUavIdx].first != nullptr)
			{
				if (rarrUAVs[nUavIdx].second == nUAV)
				{
					nFoundIdx = nUavIdx;
					break;
				}
			}
		}
		if (nFoundIdx != -1)
		{
			for (int32_t nIdx = nFoundIdx; nIdx < (rNumUAVs - 1); ++nIdx)
			{
				rarrUAVs[nIdx]				= rarrUAVs[nIdx + 1];
				m_arrUAVsWriteOffsets[nIdx] = m_arrUAVsWriteOffsets[nIdx + 1];
			}
			rarrUAVs[rNumUAVs - 1].first = nullptr;
			--rNumUAVs;
		}
		RefreshRenderTargetsAndUAVs(bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	int32_t IOERPI_Platform::GetUAVBuffer(std::array<ID3D11UnorderedAccessView *, ms_nNumUAVs> &rarrOutUAVs,
										  EUAVSet bSet)
	{
		int32_t &rNumUAVs = (bSet == EUAVSet::ComputeShader) ? m_nNumCSUAVs : m_nNumUAVs;
		auto &rarrUAVs	= bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;

		for (int i = 0; i < ms_nNumUAVs; ++i)
		{
			rarrOutUAVs[i] = nullptr;
		}

		if (rNumUAVs == 0)
		{
			return 0;
		}
		int32_t nFirstIdx = GetFirstUAVSlotIndex(bSet);
		int32_t nMaxIdx   = GetMaxUAVIndex(bSet);

		if (nMaxIdx < nFirstIdx)
		{
			return 0;
		}
		int32_t nNumExpectedUAVs = (nMaxIdx - nFirstIdx) + 1;
		for (int32_t nIdx = 0; nIdx < nNumExpectedUAVs; ++nIdx)
		{
			rarrOutUAVs[nIdx] = nullptr;
		}
		for (int32_t nIdx = 0; nIdx < rNumUAVs; ++nIdx)
		{
			int32_t nOffset		 = rarrUAVs[nIdx].second - nFirstIdx;
			rarrOutUAVs[nOffset] = rarrUAVs[nIdx].first.get();
		}
		return nNumExpectedUAVs;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetUAV(IOE::Renderer::IOETexture &rTexture, int32_t nUAVIdx /*=0*/, EUAVSet bSet,
								 int arrOffset /*=0*/)
	{
		int32_t &rNumUAVs = (bSet == EUAVSet::ComputeShader) ? m_nNumCSUAVs : m_nNumUAVs;
		auto &rarrUAVs	= bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;

		if (bSet != EUAVSet::ComputeShader)
		{
			// nUAVIdx += GetNumRenderTargets();
		}
		if (rarrUAVs[m_nNumUAVs].first == nullptr)
		{
			rarrUAVs[rNumUAVs].first  = TComPtrCast<ID3D11UnorderedAccessView>(rTexture.GetPlatform().GetViewShared());
			rarrUAVs[rNumUAVs].second = nUAVIdx;
			m_arrUAVsWriteOffsets[m_nNumUAVs] = static_cast<uint32_t>(arrOffset);
			++rNumUAVs;
		}

		RefreshRenderTargetsAndUAVs(bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetUAVs(std::vector<IOE::Renderer::IOETexture> arrTextures, EUAVSet bSet)
	{
		int32_t &rNumUAVs = (bSet == EUAVSet::ComputeShader) ? m_nNumCSUAVs : m_nNumUAVs;
		auto &rarrUAVs	= bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;
		rNumUAVs		  = 0;
		for (auto &rTexture : arrTextures)
		{
			rarrUAVs[rNumUAVs++].second = 0;
			rarrUAVs[rNumUAVs].first = TComPtrCast<ID3D11UnorderedAccessView>(rTexture.GetPlatform().GetViewShared());
		}

		RefreshRenderTargetsAndUAVs(bSet);
	}

	//////////////////////////////////////////////////////////////////////////

	int32_t IOERPI_Platform::GetNumUAVs(EUAVSet bSet) const
	{
		return bSet == EUAVSet::ComputeShader ? m_nNumCSUAVs : m_nNumUAVs;
	}

	//////////////////////////////////////////////////////////////////////////

	int32_t IOERPI_Platform::GetFirstUAVSlotIndex(EUAVSet bSet) const
	{
		int32_t nNumUAVs = GetNumUAVs(bSet);
		if (nNumUAVs <= 0)
		{
			return 0;
		}
		auto &rarrUAVs = bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;
		int32_t nFirstSlot(rarrUAVs[0].second);
		for (int32_t nIdx(1); nIdx < nNumUAVs; ++nIdx)
		{
			if (rarrUAVs[nIdx].first != nullptr)
			{
				if (rarrUAVs[nIdx].second < nFirstSlot)
				{
					nFirstSlot = rarrUAVs[nIdx].second;
				}
			}
		}
		return nFirstSlot;
	}

	//////////////////////////////////////////////////////////////////////////

	int32_t IOERPI_Platform::GetMaxUAVIndex(EUAVSet bSet) const
	{
		int32_t nNumUAVs = GetNumUAVs(bSet);
		if (nNumUAVs <= 0)
		{
			return 0;
		}
		auto &rarrUAVs = bSet == EUAVSet::ComputeShader ? m_arrCSUAVs : m_arrUAVs;
		int32_t nMaxSlot(rarrUAVs[0].second);
		for (int32_t nIdx(1); nIdx < nNumUAVs; ++nIdx)
		{
			if (rarrUAVs[nIdx].first != nullptr)
			{
				if (rarrUAVs[nIdx].second > nMaxSlot)
				{
					nMaxSlot = rarrUAVs[nIdx].second;
				}
			}
		}
		return nMaxSlot;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::RefreshRenderTargetsAndUAVs(EUAVSet bSet)
	{
		std::array<ID3D11UnorderedAccessView *, ms_nNumUAVs> arrUAVs;
		int32_t nNumUAVsToAssign = GetUAVBuffer(arrUAVs, bSet);
		int32_t nFirstUAVSlotIdx = GetFirstUAVSlotIndex(bSet);

		UINT uNumRenderTargets					  = GetNumRenderTargets();
		ID3D11DepthStencilView *pDepthStencilView = m_pD3DDepthStencilView.get();
		ID3D11UnorderedAccessView **ppUAVS		  = arrUAVs.data();

		if (bSet == EUAVSet::ComputeShader)
		{
			GetDeviceContext()->CSSetUnorderedAccessViews(
				GetFirstUAVSlotIndex(bSet), IOE::Maths::Max(nNumUAVsToAssign, 1), ppUAVS, m_arrUAVsWriteOffsets);
		}
		else
		{
			if (uNumRenderTargets == 0)
			{
				m_pD3DDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
					0, nullptr, pDepthStencilView,
					// In PS, UAVs and Render targets share same resource slots.
					// Therefore, when we bind our UAVs, we need to offset into the
					// resource array to where our UAVs will be written to, which
					// are contiguous from the Render Targets
					nFirstUAVSlotIdx, nNumUAVsToAssign, ppUAVS, m_arrUAVsWriteOffsets);
				// m_pD3DDeviceContext->OMSetRenderTargets(0, nullptr, m_pD3DDepthStencilView.get());
			}
			else
			{
				std::array<ID3D11RenderTargetView *, ms_nNumRenderTargets> arrRenderTargets;
				std::transform(
					std::begin(m_arrRenderTargets), std::end(m_arrRenderTargets), std::begin(arrRenderTargets),
					[](TComSharedPtr<ID3D11RenderTargetView> pPtr) -> ID3D11RenderTargetView * { return pPtr.get(); });
				ID3D11RenderTargetView **ppRenderTargetViews = arrRenderTargets.data();

				m_pD3DDeviceContext->OMSetRenderTargetsAndUnorderedAccessViews(
					uNumRenderTargets, ppRenderTargetViews, pDepthStencilView,
					// In PS, UAVs and Render targets share same resource slots.
					// Therefore, when we bind our UAVs, we need to offset into the
					// resource array to where our UAVs will be written to, which
					// are contiguous from the Render Targets
					nFirstUAVSlotIdx, nNumUAVsToAssign, ppUAVS, m_arrUAVsWriteOffsets);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::ResetState()
	{
		m_pD3DDeviceContext->VSSetShader(nullptr, nullptr, 0);
		m_pD3DDeviceContext->GSSetShader(nullptr, nullptr, 0);
		m_pD3DDeviceContext->DSSetShader(nullptr, nullptr, 0);
		m_pD3DDeviceContext->HSSetShader(nullptr, nullptr, 0);
		m_pD3DDeviceContext->PSSetShader(nullptr, nullptr, 0);
		m_pD3DDeviceContext->CSSetShader(nullptr, nullptr, 0);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::DrawBuffer(IOEBufferBase *pVertexBuffer, EPrimitiveTopology eTopology,
									 IOEBufferBase *pIndexBuffer /*= nullptr*/, std::int32_t nNumElements /*=-1*/)
	{
		ID3D11Buffer *pD3DVertexBuffer(pVertexBuffer->GetPlatform().GetD3DBuffer());
		ID3D11Buffer *pD3DIndexBuffer(pIndexBuffer ? pIndexBuffer->GetPlatform().GetD3DBuffer() : nullptr);

		DXGI_FORMAT tFormat(DXGI_FORMAT_R32_UINT);
		if (pIndexBuffer != nullptr)
		{
			tFormat =
				pIndexBuffer->GetElementSize() == sizeof(std::uint32_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		}

		if (pD3DVertexBuffer != nullptr)
		{
			UINT uOffset(0);
			UINT uStride((UINT)pVertexBuffer->GetElementSize());

			m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &pD3DVertexBuffer, &uStride, &uOffset);
			m_pD3DDeviceContext->IASetIndexBuffer(pD3DIndexBuffer, tFormat, 0);

			IOERPI *pRPI(GetRPI());
			// pRPI->SetPrimitiveTopology(eTopology);
			GetDeviceContext()->IASetPrimitiveTopology(GetPrimitiveFromEPrimitive(eTopology));

			if (pD3DIndexBuffer != nullptr)
			{
				UINT nNumVerts = nNumElements <= 0 ? (UINT)pIndexBuffer->GetNumElements() : (UINT)nNumElements;
				m_pD3DDeviceContext->DrawIndexed(nNumVerts, 0, 0);
			}
			else
			{
				UINT nNumVerts = nNumElements <= 0 ? (UINT)pVertexBuffer->GetNumElements() : (UINT)nNumElements;
				m_pD3DDeviceContext->Draw(nNumVerts, 0);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_CULL_MODE GetCullMode(ECullMode eCullMode)
	{
		switch (eCullMode)
		{
		case ECullMode::Front:
			return D3D11_CULL_FRONT;
		case ECullMode::Back:
			return D3D11_CULL_BACK;
		case ECullMode::None:
		default:
			return D3D11_CULL_NONE;
		}
	}

	D3D11_FILL_MODE GetFillMode(EFillMode eFillMode)
	{
		switch (eFillMode)
		{
		case EFillMode::Wireframe:
			return D3D11_FILL_WIREFRAME;
		case EFillMode::Solid:
		default:
			return D3D11_FILL_SOLID;
		}
	}

	D3D11_RASTERIZER_DESC
	GetRasterizerDescription(const IOERasterizerState &rhs)
	{
		D3D11_RASTERIZER_DESC tDesc;

		tDesc.FillMode				= GetFillMode(rhs.tData.eFillMode);
		tDesc.CullMode				= GetCullMode(rhs.tData.eCullMode);
		tDesc.FrontCounterClockwise = rhs.tData.bFrontCounterClockwise;
		tDesc.DepthBias				= rhs.tData.nDepthBias;
		tDesc.DepthBiasClamp		= rhs.tData.fDepthBiasClamp;
		tDesc.SlopeScaledDepthBias  = rhs.tData.fSlopeScaledDepthBias;
		tDesc.DepthClipEnable		= rhs.tData.bDepthClipEnable;
		tDesc.ScissorEnable			= rhs.tData.bScissorEnable;
		tDesc.MultisampleEnable		= rhs.tData.bMultisampleEnable;
		tDesc.AntialiasedLineEnable = rhs.tData.bAntialiasedLineEnable;

		return tDesc;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetRasterizerState(const IOERasterizerState &rhs)
	{
		IOE_ASSERT(rhs.GetPlatform().GetD3D11State() != nullptr, "Rasterizer state not yet set up!");
		GetDeviceContext()->RSSetState(const_cast<ID3D11RasterizerState *>(rhs.GetPlatform().GetD3D11State()));
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERasterizerState_Platform::Setup(IOERPI *pRPI)
	{
		D3D11_RASTERIZER_DESC tDesc(GetRasterizerDescription(GetState()));
		pRPI->GetPlatform().GetDevice()->CreateRasterizerState(&tDesc, &m_pRasterizerState);
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_BLEND GetBlendValue(EBlendValue eBlendValue)
	{
		switch (eBlendValue)
		{
		case EBlendValue::Zero:
			return D3D11_BLEND_ZERO;
		case EBlendValue::One:
			return D3D11_BLEND_ONE;
		case EBlendValue::SrcColour:
			return D3D11_BLEND_SRC_COLOR;
		case EBlendValue::InvSrcColour:
			return D3D11_BLEND_INV_SRC_COLOR;
		case EBlendValue::SrcAlpha:
			return D3D11_BLEND_SRC_ALPHA;
		case EBlendValue::InvSrcAlpha:
			return D3D11_BLEND_INV_SRC_ALPHA;
		case EBlendValue::DestAlpha:
			return D3D11_BLEND_DEST_ALPHA;
		case EBlendValue::InvDestAlpha:
			return D3D11_BLEND_INV_DEST_ALPHA;
		case EBlendValue::DestColour:
			return D3D11_BLEND_DEST_COLOR;
		case EBlendValue::InvDestColour:
			return D3D11_BLEND_INV_DEST_COLOR;
		case EBlendValue::SrcAlphaSat:
			return D3D11_BLEND_SRC_ALPHA_SAT;
		case EBlendValue::BlendFactor:
			return D3D11_BLEND_BLEND_FACTOR;
		case EBlendValue::InvBlendFactor:
			return D3D11_BLEND_INV_BLEND_FACTOR;
		default:
			return D3D11_BLEND_ZERO;
		}
	}

	D3D11_BLEND_OP GetBlendOperation(EBlendOperation eOperation)
	{
		switch (eOperation)
		{
		case EBlendOperation::Add:
			return D3D11_BLEND_OP_ADD;
		case EBlendOperation::Subtract:
			return D3D11_BLEND_OP_SUBTRACT;
		case EBlendOperation::ReverseSubtract:
			return D3D11_BLEND_OP_REV_SUBTRACT;
		case EBlendOperation::Min:
			return D3D11_BLEND_OP_MIN;
		case EBlendOperation::Max:
			return D3D11_BLEND_OP_MAX;
		default:
			return D3D11_BLEND_OP_ADD;
		}
	}

	D3D11_BLEND_DESC GetBlendDescription(const IOEBlendState &rhs)
	{
		D3D11_BLEND_DESC tDesc;

		tDesc.AlphaToCoverageEnable  = FALSE;
		tDesc.IndependentBlendEnable = FALSE;

		tDesc.RenderTarget[0].BlendEnable = rhs.tData.bBlendEnable ? TRUE : FALSE;

		tDesc.RenderTarget[0].SrcBlend  = GetBlendValue(rhs.tData.eSrcBlend);
		tDesc.RenderTarget[0].DestBlend = GetBlendValue(rhs.tData.eDestBlend);
		tDesc.RenderTarget[0].BlendOp   = GetBlendOperation(rhs.tData.eBlendOp);

		tDesc.RenderTarget[0].SrcBlendAlpha  = GetBlendValue(rhs.tData.eSrcBlendAlpha);
		tDesc.RenderTarget[0].DestBlendAlpha = GetBlendValue(rhs.tData.eDestBlendAlpha);
		tDesc.RenderTarget[0].BlendOpAlpha   = GetBlendOperation(rhs.tData.eBlendOpAlpha);

		tDesc.RenderTarget[0].RenderTargetWriteMask = rhs.tData.nRenderTargetWriteMask;

		return tDesc;
	}

	void IOEBlendState_Platform::Setup(IOERPI *pRPI)
	{
		D3D11_BLEND_DESC tDesc(GetBlendDescription(GetState()));
		pRPI->GetPlatform().GetDevice()->CreateBlendState(&tDesc, &m_pBlendState);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetBlendState(const IOEBlendState &rhs, std::uint32_t uWriteMask /* = 0xFFFFFFFF*/,
										const IOEVector &arrBlendFactor /* = Maths::g_vOneVector*/)
	{
		IOE_ASSERT(rhs.GetPlatform().GetD3D11State() != nullptr, "Blend state not yet set up!");

		float arrBlendFactor_[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		arrBlendFactor.Get(arrBlendFactor_[0], arrBlendFactor_[1], arrBlendFactor_[2], arrBlendFactor_[3]);

		GetDeviceContext()->OMSetBlendState(const_cast<ID3D11BlendState *>(rhs.GetPlatform().GetD3D11State()),
											arrBlendFactor_, uWriteMask);
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_FILTER GetSamplerFilter(ESamplerFilter eFilter)
	{
		switch (eFilter)
		{
		case ESamplerFilter::MinMagPointMipLinear:
			return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MinPointMagLinearMipPoint:
			return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MinPointMagMipLinear:
			return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
		case ESamplerFilter::MinLinearMagMipPoint:
			return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
		case ESamplerFilter::MinLinearMagPointMipLinear:
			return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MinMagLinearMipPoint:
			return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MinMagMipLinear:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		case ESamplerFilter::Anisotropic:
			return D3D11_FILTER_ANISOTROPIC;
		case ESamplerFilter::ComparisonMinMagMipPoint:
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		case ESamplerFilter::ComparisonMinMagPointMipLinear:
			return D3D11_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::ComparisonMinPointMagLinearMipPoint:
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::ComparisonMinPointMagMipLinear:
			return D3D11_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
		case ESamplerFilter::ComparisonMinLinearMagMipPoint:
			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
		case ESamplerFilter::ComparisonMinLinearMagPointMipLinear:
			return D3D11_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::ComparisonMinMagLinearMipPoint:
			return D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::ComparisonMinMagMipLinear:
			return D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		case ESamplerFilter::ComparisonAnisotropic:
			return D3D11_FILTER_COMPARISON_ANISOTROPIC;
		case ESamplerFilter::MinimumMinMagMipPoint:
			return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_POINT;
		case ESamplerFilter::MinimumMinMagPointMipLinear:
			return D3D11_FILTER_MINIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MinimumMinPointMagLinearMipPoint:
			return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MinimumMinPointMagMipLinear:
			return D3D11_FILTER_MINIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case ESamplerFilter::MinimumMinLinearMagMipPoint:
			return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case ESamplerFilter::MinimumMinLinearMagPointMipLinear:
			return D3D11_FILTER_MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MinimumMinMagLinearMipPoint:
			return D3D11_FILTER_MINIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MinimumMinMagMipLinear:
			return D3D11_FILTER_MINIMUM_MIN_MAG_MIP_LINEAR;
		case ESamplerFilter::MinimumAnisotropic:
			return D3D11_FILTER_MINIMUM_ANISOTROPIC;
		case ESamplerFilter::MaximumMinMagMipPoint:
			return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_POINT;
		case ESamplerFilter::MaximumMinMagPointMipLinear:
			return D3D11_FILTER_MAXIMUM_MIN_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MaximumMinPointMagLinearMipPoint:
			return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MaximumMinPointMagMipLinear:
			return D3D11_FILTER_MAXIMUM_MIN_POINT_MAG_MIP_LINEAR;
		case ESamplerFilter::MaximumMinLinearMagMipPoint:
			return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_MIP_POINT;
		case ESamplerFilter::MaximumMinLinearMagPointMipLinear:
			return D3D11_FILTER_MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
		case ESamplerFilter::MaximumMinMagLinearMipPoint:
			return D3D11_FILTER_MAXIMUM_MIN_MAG_LINEAR_MIP_POINT;
		case ESamplerFilter::MaximumMinMagMipLinear:
			return D3D11_FILTER_MAXIMUM_MIN_MAG_MIP_LINEAR;
		case ESamplerFilter::MaximumAnisotropic:
			return D3D11_FILTER_MAXIMUM_ANISOTROPIC;
		}
		return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}

	D3D11_TEXTURE_ADDRESS_MODE GetSamplerAddressMode(ESamplerAddressMode eMode)
	{
		switch (eMode)
		{
		case ESamplerAddressMode::Wrap:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case ESamplerAddressMode::Mirror:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		case ESamplerAddressMode::Clamp:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case ESamplerAddressMode::Border:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case ESamplerAddressMode::MirrorOnce:
			return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
		}
		return D3D11_TEXTURE_ADDRESS_CLAMP;
	}

	D3D11_COMPARISON_FUNC GetComparisonFunction(EComparisonFunction eFunction)
	{
		switch (eFunction)
		{
		case EComparisonFunction::Never:
			return D3D11_COMPARISON_NEVER;
		case EComparisonFunction::Less:
			return D3D11_COMPARISON_LESS;
		case EComparisonFunction::Equal:
			return D3D11_COMPARISON_EQUAL;
		case EComparisonFunction::LessEqual:
			return D3D11_COMPARISON_LESS_EQUAL;
		case EComparisonFunction::Greater:
			return D3D11_COMPARISON_GREATER;
		case EComparisonFunction::NotEqual:
			return D3D11_COMPARISON_NOT_EQUAL;
		case EComparisonFunction::GreaterEqual:
			return D3D11_COMPARISON_GREATER_EQUAL;
		case EComparisonFunction::Always:
			return D3D11_COMPARISON_ALWAYS;
		}
		return D3D11_COMPARISON_NEVER;
	}

	D3D11_SAMPLER_DESC GetSamplerDescription(const IOESamplerState &rhs)
	{
		D3D11_SAMPLER_DESC tDesc;

		tDesc.Filter		 = GetSamplerFilter(rhs.tData.eFilter);
		tDesc.AddressU		 = GetSamplerAddressMode(rhs.tData.eAddressModeU);
		tDesc.AddressV		 = GetSamplerAddressMode(rhs.tData.eAddressModeV);
		tDesc.AddressW		 = GetSamplerAddressMode(rhs.tData.eAddressModeW);
		tDesc.MipLODBias	 = rhs.tData.fLODBias;
		tDesc.MaxAnisotropy  = rhs.tData.uMaxAnisotropy;
		tDesc.ComparisonFunc = GetComparisonFunction(rhs.tData.eComparison);
		rhs.tData.vBorderColour.Get(tDesc.BorderColor[0], tDesc.BorderColor[1], tDesc.BorderColor[2],
									tDesc.BorderColor[3]);
		tDesc.MinLOD = rhs.tData.fMinLOD;
		tDesc.MaxLOD = rhs.tData.fMaxLOD;

		return tDesc;
	}

	void IOESamplerState_Platform::Setup(IOERPI *pRPI)
	{
		D3D11_SAMPLER_DESC tDesc(GetSamplerDescription(GetState()));
		pRPI->GetPlatform().GetDevice()->CreateSamplerState(&tDesc, &m_pSamplerState);
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_STENCIL_OP GetStencilOperation(EStencilOperation eOperation)
	{
		switch (eOperation)
		{
		case EStencilOperation::Keep:
			return D3D11_STENCIL_OP_KEEP;
		case EStencilOperation::Zero:
			return D3D11_STENCIL_OP_ZERO;
		case EStencilOperation::Replace:
			return D3D11_STENCIL_OP_REPLACE;
		case EStencilOperation::IncrementSaturate:
			return D3D11_STENCIL_OP_INCR_SAT;
		case EStencilOperation::DecrementSaturate:
			return D3D11_STENCIL_OP_DECR_SAT;
		case EStencilOperation::Invert:
			return D3D11_STENCIL_OP_INVERT;
		case EStencilOperation::Increment:
			return D3D11_STENCIL_OP_INCR;
		case EStencilOperation::Decrement:
		default:
			return D3D11_STENCIL_OP_DECR;
		}
	}

	D3D11_DEPTH_STENCIL_DESC GetDepthDescription(const IOEDepthState &rhs)
	{
		D3D11_DEPTH_STENCIL_DESC tDesc;

		tDesc.DepthEnable	= rhs.tData.bDepthEnable;
		tDesc.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)rhs.tData.uDepthWriteMask;
		tDesc.DepthFunc		 = GetComparisonFunction(rhs.tData.eDepthFunc);

		tDesc.StencilEnable	= rhs.tData.bStencilEnable;
		tDesc.StencilReadMask  = rhs.tData.uStencilReadMask;
		tDesc.StencilWriteMask = rhs.tData.uStencilWriteMask;

		tDesc.FrontFace.StencilFailOp	  = GetStencilOperation(rhs.tData.tFrontFace.eStencilFailOp);
		tDesc.FrontFace.StencilDepthFailOp = GetStencilOperation(rhs.tData.tFrontFace.eStencilDepthFailOp);
		tDesc.FrontFace.StencilPassOp	  = GetStencilOperation(rhs.tData.tFrontFace.eStencilPassOp);
		tDesc.FrontFace.StencilFunc		   = GetComparisonFunction(rhs.tData.tFrontFace.eStencilFunc);

		tDesc.BackFace.StencilFailOp	  = GetStencilOperation(rhs.tData.tRearFace.eStencilFailOp);
		tDesc.BackFace.StencilDepthFailOp = GetStencilOperation(rhs.tData.tRearFace.eStencilDepthFailOp);
		tDesc.BackFace.StencilPassOp	  = GetStencilOperation(rhs.tData.tRearFace.eStencilPassOp);
		tDesc.BackFace.StencilFunc		  = GetComparisonFunction(rhs.tData.tRearFace.eStencilFunc);

		return tDesc;
	}

	void IOEDepthState_Platform::Setup(IOERPI *pRPI)
	{
		D3D11_DEPTH_STENCIL_DESC tDesc(GetDepthDescription(GetState()));
		pRPI->GetPlatform().GetDevice()->CreateDepthStencilState(&tDesc, &m_pDepthState);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetSamplerState(const IOESamplerState &rhs)
	{
		IOE_ASSERT(rhs.GetPlatform().GetD3D11State() != nullptr, "Sampler state not yet set up!");

		IOERPI *pRPI(GetRPI());

		ID3D11SamplerState *pSamplerState(const_cast<ID3D11SamplerState *>(rhs.GetPlatform().GetD3D11State()));
		pRPI->GetPlatform().GetDeviceContext()->CSSetSamplers(0, 1, &pSamplerState);
		pRPI->GetPlatform().GetDeviceContext()->VSSetSamplers(0, 1, &pSamplerState);
		pRPI->GetPlatform().GetDeviceContext()->DSSetSamplers(0, 1, &pSamplerState);
		pRPI->GetPlatform().GetDeviceContext()->HSSetSamplers(0, 1, &pSamplerState);
		pRPI->GetPlatform().GetDeviceContext()->GSSetSamplers(0, 1, &pSamplerState);
		pRPI->GetPlatform().GetDeviceContext()->PSSetSamplers(0, 1, &pSamplerState);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetDepthState(const IOEDepthState &rhs, std::uint32_t uStencilRef /*=0*/)
	{
		IOE_ASSERT(rhs.GetPlatform().GetD3D11State() != nullptr, "Sampler state not yet set up!");

		IOERPI *pRPI(GetRPI());

		ID3D11DepthStencilState *pDepthState(const_cast<ID3D11DepthStencilState *>(rhs.GetPlatform().GetD3D11State()));

		pRPI->GetPlatform().GetDeviceContext()->OMSetDepthStencilState(pDepthState, uStencilRef);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetProfileMarker(const std::string &szProfilerMark, EProfileMarkerType eProfileMarker,
										   const IOE::Maths::IOEVector &vColour)
	{
#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)
		std::wstring wszProfileMarker(IOE::Core::Algorithm::ConvertNarrowToWide(szProfilerMark));

		float fX, fY, fZ, fW;
		vColour.Get(fX, fY, fZ, fW);

		DWORD uColour(D3DCOLOR_COLORVALUE(fX, fY, fZ, fW));
		if (eProfileMarker == EProfileMarkerType::Start)
		{
			if (m_pPerfAnnotations != nullptr)
			{
				m_pPerfAnnotations->BeginEvent(wszProfileMarker.c_str());
			}
			else
			{
				D3DPERF_BeginEvent(uColour, wszProfileMarker.c_str());
			}
		}
		else if (eProfileMarker == EProfileMarkerType::End)
		{
			if (m_pPerfAnnotations != nullptr)
			{
				m_pPerfAnnotations->EndEvent();
			}
			else
			{
				D3DPERF_EndEvent();
			}
		}
		else if (eProfileMarker == EProfileMarkerType::Single)
		{
			if (m_pPerfAnnotations != nullptr)
			{
				m_pPerfAnnotations->SetMarker(wszProfileMarker.c_str());
			}
			else
			{
				D3DPERF_SetMarker(uColour, wszProfileMarker.c_str());
			}
		}
#endif
	}

	//////////////////////////////////////////////////////////////////////////

	void IOERPI_Platform::SetPrimitiveTopology(EPrimitiveTopology eTopology)
	{
		D3D11_PRIMITIVE_TOPOLOGY tTopology(GetPrimitiveFromEPrimitive(eTopology));

		GetDeviceContext()->IASetPrimitiveTopology(tTopology);
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE