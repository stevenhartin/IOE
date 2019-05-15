#include <d3d12.h>

#include <IOE/IOECore/IOEManagerContainer.h>

#include <IOE/IOEMaths/Math.h>

#include "IOETexture_Platform.h"

#include "../IOERPI.h"
#include "../IOETexture.h"
#include "../IOEBuffer.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	DXGI_FORMAT GetFormat(ETextureFormat eFormat)
	{
		switch (eFormat)
		{
		case ETextureFormat::R32G32B32A32_TYPELESS:
			return DXGI_FORMAT_R32G32B32A32_TYPELESS;
		case ETextureFormat::R32G32B32A32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case ETextureFormat::R32G32B32A32_UINT:
			return DXGI_FORMAT_R32G32B32A32_UINT;
		case ETextureFormat::R32G32B32A32_SINT:
			return DXGI_FORMAT_R32G32B32A32_SINT;
		case ETextureFormat::R32G32B32_TYPELESS:
			return DXGI_FORMAT_R32G32B32_TYPELESS;
		case ETextureFormat::R32G32B32_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case ETextureFormat::R32G32B32_UINT:
			return DXGI_FORMAT_R32G32B32_UINT;
		case ETextureFormat::R32G32B32_SINT:
			return DXGI_FORMAT_R32G32B32_SINT;
		case ETextureFormat::R16G16B16A16_TYPELESS:
			return DXGI_FORMAT_R16G16B16A16_TYPELESS;
		case ETextureFormat::R16G16B16A16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case ETextureFormat::R16G16B16A16_UNORM:
			return DXGI_FORMAT_R16G16B16A16_UNORM;
		case ETextureFormat::R16G16B16A16_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;
		case ETextureFormat::R16G16B16A16_SNORM:
			return DXGI_FORMAT_R16G16B16A16_SNORM;
		case ETextureFormat::R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		case ETextureFormat::R32G32_TYPELESS:
			return DXGI_FORMAT_R32G32_TYPELESS;
		case ETextureFormat::R32G32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
		case ETextureFormat::R32G32_UINT:
			return DXGI_FORMAT_R32G32_UINT;
		case ETextureFormat::R32G32_SINT:
			return DXGI_FORMAT_R32G32_SINT;
		case ETextureFormat::R32G8X24_TYPELESS:
			return DXGI_FORMAT_R32G8X24_TYPELESS;
		case ETextureFormat::D32_FLOAT_S8X24_UINT:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		case ETextureFormat::R32_FLOAT_X8X24_TYPELESS:
			return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		case ETextureFormat::X32_TYPELESS_G8X24_UINT:
			return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
		case ETextureFormat::R10G10B10A2_TYPELESS:
			return DXGI_FORMAT_R10G10B10A2_TYPELESS;
		case ETextureFormat::R10G10B10A2_UNORM:
			return DXGI_FORMAT_R10G10B10A2_UNORM;
		case ETextureFormat::R10G10B10A2_UINT:
			return DXGI_FORMAT_R10G10B10A2_UINT;
		case ETextureFormat::R11G11B10_FLOAT:
			return DXGI_FORMAT_R11G11B10_FLOAT;
		case ETextureFormat::R8G8B8A8_TYPELESS:
			return DXGI_FORMAT_R8G8B8A8_TYPELESS;
		case ETextureFormat::R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case ETextureFormat::R8G8B8A8_UNORM_SRGB:
			return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		case ETextureFormat::R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case ETextureFormat::R8G8B8A8_SNORM:
			return DXGI_FORMAT_R8G8B8A8_SNORM;
		case ETextureFormat::R8G8B8A8_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case ETextureFormat::R16G16_TYPELESS:
			return DXGI_FORMAT_R16G16_TYPELESS;
		case ETextureFormat::R16G16_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
		case ETextureFormat::R16G16_UNORM:
			return DXGI_FORMAT_R16G16_UNORM;
		case ETextureFormat::R16G16_UINT:
			return DXGI_FORMAT_R16G16_UINT;
		case ETextureFormat::R16G16_SNORM:
			return DXGI_FORMAT_R16G16_SNORM;
		case ETextureFormat::R16G16_SINT:
			return DXGI_FORMAT_R16G16_SINT;
		case ETextureFormat::R32_TYPELESS:
			return DXGI_FORMAT_R32_TYPELESS;
		case ETextureFormat::D32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;
		case ETextureFormat::R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case ETextureFormat::R32_UINT:
			return DXGI_FORMAT_R32_UINT;
		case ETextureFormat::R32_SINT:
			return DXGI_FORMAT_R32_SINT;
		case ETextureFormat::R24G8_TYPELESS:
			return DXGI_FORMAT_R24G8_TYPELESS;
		case ETextureFormat::D24_UNORM_S8_UINT:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case ETextureFormat::R24_UNORM_X8_TYPELESS:
			return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		case ETextureFormat::X24_TYPELESS_G8_UINT:
			return DXGI_FORMAT_X24_TYPELESS_G8_UINT;
		case ETextureFormat::R8G8_TYPELESS:
			return DXGI_FORMAT_R8G8_TYPELESS;
		case ETextureFormat::R8G8_UNORM:
			return DXGI_FORMAT_R8G8_UNORM;
		case ETextureFormat::R8G8_UINT:
			return DXGI_FORMAT_R8G8_UINT;
		case ETextureFormat::R8G8_SNORM:
			return DXGI_FORMAT_R8G8_SNORM;
		case ETextureFormat::R8G8_SINT:
			return DXGI_FORMAT_R8G8_SINT;
		case ETextureFormat::R16_TYPELESS:
			return DXGI_FORMAT_R16_TYPELESS;
		case ETextureFormat::R16_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
		case ETextureFormat::D16_UNORM:
			return DXGI_FORMAT_D16_UNORM;
		case ETextureFormat::R16_UNORM:
			return DXGI_FORMAT_R16_UNORM;
		case ETextureFormat::R16_UINT:
			return DXGI_FORMAT_R16_UINT;
		case ETextureFormat::R16_SNORM:
			return DXGI_FORMAT_R16_SNORM;
		case ETextureFormat::R16_SINT:
			return DXGI_FORMAT_R16_SINT;
		case ETextureFormat::R8_TYPELESS:
			return DXGI_FORMAT_R8_TYPELESS;
		case ETextureFormat::R8_UNORM:
			return DXGI_FORMAT_R8_UNORM;
		case ETextureFormat::R8_UINT:
			return DXGI_FORMAT_R8_UINT;
		case ETextureFormat::R8_SNORM:
			return DXGI_FORMAT_R8_SNORM;
		case ETextureFormat::R8_SINT:
			return DXGI_FORMAT_R8_SINT;
		case ETextureFormat::A8_UNORM:
			return DXGI_FORMAT_A8_UNORM;
		case ETextureFormat::R1_UNORM:
			return DXGI_FORMAT_R1_UNORM;
		case ETextureFormat::B8G8R8A8_UNORM:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		default:
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture_Platform::IOETexture_Platform(IOETexture &rTexture)
		: IOETexture_PlatformBase(rTexture)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOETexture_Platform::~IOETexture_Platform()
	{
		m_pTexture.reset();
		m_pStagingTexture.reset();
		m_pView.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::GetDescription1D(D3D11_TEXTURE1D_DESC &rDesc,
											   std::uint32_t uNumMipMaps,
											   bool bStaging /*=false*/) const
	{
		if (GetTexture().GetTextureType() == ETextureType::Texture1D ||
			GetTexture().GetTextureType() == ETextureType::Texture1DArray)
		{
			rDesc.Width = static_cast<UINT>(GetTexture().GetWidth());
			rDesc.MipLevels = uNumMipMaps;
			rDesc.ArraySize = static_cast<UINT>(GetTexture().GetArrayLength());
			rDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			rDesc.Usage =
				GetCPUAccess(GetTexture().GetBufferAccess(), bStaging);
			rDesc.BindFlags = GetBindFlag(GetTexture().GetBufferDescription());
			rDesc.CPUAccessFlags =
				GetTextureAccessType(GetTexture().GetBufferAccess(), bStaging);
			rDesc.MiscFlags = 0;

			if (bStaging)
			{
				rDesc.BindFlags = 0;
			}

			if (rDesc.Width <= 0)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::GetDescription2D(D3D11_TEXTURE2D_DESC &rDesc,
											   std::uint32_t uNumMipMaps,
											   bool bStaging /*=false*/) const
	{
		if (GetTexture().GetTextureType() == ETextureType::Texture2D ||
			GetTexture().GetTextureType() == ETextureType::Texture2DArray)
		{
			rDesc.Width = static_cast<UINT>(GetTexture().GetWidth());
			rDesc.Height = static_cast<UINT>(GetTexture().GetHeight());
			rDesc.MipLevels = uNumMipMaps;
			rDesc.ArraySize = static_cast<UINT>(GetTexture().GetArrayLength());
			rDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			rDesc.SampleDesc.Count   = 1;
			rDesc.SampleDesc.Quality = 0;
			rDesc.Usage =
				GetCPUAccess(GetTexture().GetBufferAccess(), bStaging);
			rDesc.BindFlags = GetBindFlag(GetTexture().GetBufferDescription());
			rDesc.CPUAccessFlags =
				GetTextureAccessType(GetTexture().GetBufferAccess(), bStaging);
			rDesc.MiscFlags = 0;

			if (bStaging)
			{
				rDesc.BindFlags = 0;
			}

			if (rDesc.Width <= 0 || rDesc.Height <= 0)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::GetDescription3D(D3D11_TEXTURE3D_DESC &rDesc,
											   std::uint32_t uNumMipMaps,
											   bool bStaging /*=false*/) const
	{
		if (GetTexture().GetTextureType() == ETextureType::Texture3D)
		{
			rDesc.Width = static_cast<UINT>(GetTexture().GetWidth());
			rDesc.Height = static_cast<UINT>(GetTexture().GetHeight());
			rDesc.Depth = static_cast<UINT>(GetTexture().GetDepth());
			rDesc.MipLevels = uNumMipMaps;
			rDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			rDesc.Usage =
				GetCPUAccess(GetTexture().GetBufferAccess(), bStaging);
			rDesc.BindFlags = GetBindFlag(GetTexture().GetBufferDescription());
			rDesc.CPUAccessFlags =
				GetTextureAccessType(GetTexture().GetBufferAccess(), bStaging);
			rDesc.MiscFlags = 0;

			if (bStaging)
			{
				rDesc.BindFlags = 0;
			}

			if (rDesc.Width <= 0 || rDesc.Height <= 0 || rDesc.Depth <= 0)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool
	IOETexture_Platform::GetDescriptionCube(D3D11_TEXTURE2D_DESC &rDesc,
											std::uint32_t uNumMipMaps,
											bool bStaging /*=false*/) const
	{
		if (GetTexture().GetTextureType() == ETextureType::TextureCube)
		{
			rDesc.Width = static_cast<UINT>(GetTexture().GetWidth());
			rDesc.Height = static_cast<UINT>(GetTexture().GetHeight());
			rDesc.MipLevels = uNumMipMaps;
			rDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			rDesc.Usage =
				GetCPUAccess(GetTexture().GetBufferAccess(), bStaging);
			rDesc.BindFlags = GetBindFlag(GetTexture().GetBufferDescription());
			rDesc.CPUAccessFlags =
				GetTextureAccessType(GetTexture().GetBufferAccess(), bStaging);
			rDesc.ArraySize = 6 *
				Maths::Max(static_cast<UINT>(GetTexture().GetArrayLength()),
						   1U);
			rDesc.SampleDesc.Quality = 0;
			rDesc.SampleDesc.Count   = 1;
			rDesc.MiscFlags			 = D3D11_RESOURCE_MISC_TEXTURECUBE;

			if (bStaging)
			{
				rDesc.BindFlags = 0;
			}

			if (rDesc.Width <= 0 || rDesc.Height <= 0)
			{
				return false;
			}

			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool
	IOETexture_Platform::GetDescriptionBuffer(D3D11_BUFFER_DESC &rDesc,
											  bool bStaging /*=false*/) const
	{
		if (GetTexture().GetTextureType() == ETextureType::GenericBuffer)
		{
			memset(&rDesc, 0, sizeof(rDesc));

			if (GetTexture().CanAllowRawValues())
			{
				// Raw values ignores byte stride and uses byte width
				rDesc.StructureByteStride = 0;
				rDesc.ByteWidth = static_cast<UINT>(GetTexture().GetWidth()) *
					static_cast<UINT>(GetTexture().GetArrayLength());
				if (!bStaging)
					rDesc.MiscFlags =
						D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
			}
			else
			{
				rDesc.StructureByteStride =
					static_cast<UINT>(GetTexture().GetWidth());
				rDesc.ByteWidth =
					static_cast<UINT>(GetTexture().GetArrayLength()) *
					rDesc.StructureByteStride;
				if (!bStaging)
					rDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			}
			rDesc.BindFlags = GetBindFlag(GetTexture().GetBufferDescription());
			rDesc.Usage =
				GetCPUAccess(GetTexture().GetBufferAccess(), bStaging);
			rDesc.CPUAccessFlags =
				GetTextureAccessType(GetTexture().GetBufferAccess(), bStaging);
			if (bStaging)
			{
				rDesc.BindFlags = 0;
			}
			if (rDesc.ByteWidth <= 0)
			{
				return false;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::MapBuffer(EBufferMapType eMapType, void *&rpData,
										std::uint32_t uSubresourceID /*=0*/)
	{
		ID3D11DeviceContext *pDeviceContext(
			IOE::Renderer::IOERPI::GetSingletonPtr()
				->GetPlatform()
				.GetDeviceContext());

		ID3D11Resource *pTexture(m_pTexture.get());

		if (m_pStagingTexture != nullptr &&
			GetTexture().GetBufferAccess() == ECPUAccess::GPUWriteBack)
		{
			pDeviceContext->CopyResource(m_pStagingTexture.get(),
										 m_pTexture.get());

			pTexture = m_pStagingTexture.get();
		}

		D3D11_MAPPED_SUBRESOURCE tMappedSubresource;
		if (SUCCEEDED(pDeviceContext->Map(pTexture, uSubresourceID,
										  GetMapType(eMapType), uSubresourceID,
										  &tMappedSubresource)))
		{
			rpData = tMappedSubresource.pData;
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::UnmapBuffer(std::uint32_t uSubresourceID /*=0*/)
	{
		ID3D11DeviceContext *pDeviceContext(
			IOE::Renderer::IOERPI::GetSingletonPtr()
				->GetPlatform()
				.GetDeviceContext());

		ID3D11Resource *pTexture(m_pTexture.get());

		if (m_pStagingTexture != nullptr &&
			GetTexture().GetBufferAccess() == ECPUAccess::GPUWriteBack)
		{
			pTexture = m_pStagingTexture.get();
		}

		pDeviceContext->Unmap(pTexture, uSubresourceID);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::OnResize()
	{
		Setup();
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::Setup(IOETextureSource *pSource /*=nullptr*/)
	{
		Cleanup();
		if (CreateTexture(pSource))
		{
			bool bSuccess(true);
			if (GetTexture().GetBufferAccess() == ECPUAccess::GPUWriteBack)
			{
				bSuccess = CreateStagingTexture(pSource);
			}
			if (bSuccess && CreateView())
			{
				return true;
			}
		}
		IOE_ASSERT(IOE::Renderer::IOERPI::GetSingletonPtr()
						   ->GetPlatform()
						   .GetDevice() == nullptr,
				   "Failed to setup texture");
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool
	IOETexture_Platform::CreateTexture(IOETextureSource *pSource /*=nullptr*/)
	{
		return CreateTextureInternal(m_pTexture, pSource, false);
	}

	bool IOETexture_Platform::CreateStagingTexture(
		IOETextureSource *pSource /*=nullptr*/)
	{
		return CreateTextureInternal(m_pStagingTexture, pSource, true);
	}

	bool IOETexture_Platform::CreateTextureInternal(
		TComSharedPtr<ID3D11Resource> &rpResource,
		IOETextureSource *pSource /*=nullptr*/, bool bStaging /*=false*/)
	{
		ID3D11Device *pDevice(IOE::Renderer::IOERPI::GetSingletonPtr()
								  ->GetPlatform()
								  .GetDevice());
		rpResource.reset();
		if (pDevice == nullptr)
		{
			return false;
		}

		std::uint32_t uNumMipMaps(GetTexture().GetNumMips());
		std::unique_ptr<D3D11_SUBRESOURCE_DATA[]> pSubResouce;

		if (pSource != nullptr)
		{
			GetTexture().SetNumMips(pSource->uNumMipMaps);
			uNumMipMaps = pSource->uNumMipMaps;
			pSubResouce =
				std::make_unique<D3D11_SUBRESOURCE_DATA[]>(uNumMipMaps);

			for (uint32_t uIdx(0); uIdx < uNumMipMaps; ++uIdx)
			{
				D3D11_SUBRESOURCE_DATA &rSubResource(pSubResouce.get()[uIdx]);

				// TODO: Calculate these subresources properly for textures
				// which have been loaded
				// by files. Right now, we don't support mipmapping.
				rSubResource.pSysMem		  = pSource->pSrc;
				rSubResource.SysMemPitch	  = 0;
				rSubResource.SysMemSlicePitch = 0;

				switch (GetTexture().GetTextureType())
				{
				case ETextureType::Texture3D:
					rSubResource.SysMemSlicePitch = static_cast<UINT>(
						GetTexture().GetWidth() * GetTexture().GetHeight() *
						pSource->uPixelSizeBytes);
				// intentional fall through
				case ETextureType::Texture2D:

					rSubResource.SysMemPitch = static_cast<UINT>(
						GetTexture().GetWidth() * pSource->uPixelSizeBytes);
					break;
				}
			}
		}

		if (GetTexture().IsDefaultRenderTarget())
		{
			// The default render target is the buffer in the swap chain
			IDXGISwapChain *pSwapChain(IOE::Renderer::IOERPI::GetSingletonPtr()
										   ->GetPlatform()
										   .GetSwapChain());
			if (pSwapChain == nullptr)
			{
				return false;
			}
			ID3D11Buffer *pBuffer = nullptr;
			if (FAILED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
											 (void **)&pBuffer)))
			{
				return false;
			}
			rpResource = pBuffer;
			return true;
		}
		if (GetTexture().GetTextureType() == ETextureType::Texture1D ||
			GetTexture().GetTextureType() == ETextureType::Texture1DArray)
		{
			D3D11_TEXTURE1D_DESC tDesc;
			if (GetDescription1D(tDesc, uNumMipMaps, bStaging))
			{
				ID3D11Texture1D *pTexture = nullptr;
				if (FAILED(pDevice->CreateTexture1D(&tDesc, pSubResouce.get(),
													&pTexture)) ||
					pTexture == nullptr)
				{
					return false;
				}
				rpResource = pTexture;
				return true;
			}
		}
		else if (GetTexture().GetTextureType() == ETextureType::Texture2D ||
				 GetTexture().GetTextureType() == ETextureType::Texture2DArray)
		{
			D3D11_TEXTURE2D_DESC tDesc;
			if (GetDescription2D(tDesc, uNumMipMaps, bStaging))
			{
				ID3D11Texture2D *pTexture = nullptr;
				BOOL bTest = FAILED(pDevice->CreateTexture2D(
					&tDesc, pSubResouce.get(), &pTexture));
				if (bTest || pTexture == nullptr)
				{
					return false;
				}
				rpResource = pTexture;
				return true;
			}
		}
		else if (GetTexture().GetTextureType() == ETextureType::Texture3D)
		{
			D3D11_TEXTURE3D_DESC tDesc;
			if (GetDescription3D(tDesc, uNumMipMaps, bStaging))
			{
				ID3D11Texture3D *pTexture = nullptr;
				if (FAILED(pDevice->CreateTexture3D(&tDesc, pSubResouce.get(),
													&pTexture)) ||
					pTexture == nullptr)
				{
					return false;
				}
				rpResource = pTexture;
				return true;
			}
		}
		else if (GetTexture().GetTextureType() == ETextureType::TextureCube ||
				 GetTexture().GetTextureType() ==
					 ETextureType::TextureCubeArray)
		{
			D3D11_TEXTURE2D_DESC tDesc;
			if (GetDescriptionCube(tDesc, uNumMipMaps, bStaging))
			{
				ID3D11Texture2D *pTexture = nullptr;
				if (FAILED(pDevice->CreateTexture2D(&tDesc, pSubResouce.get(),
													&pTexture)) ||
					pTexture == nullptr)
				{
					return false;
				}
				rpResource = pTexture;
				return true;
			}
		}
		else if (GetTexture().GetTextureType() == ETextureType::GenericBuffer)
		{
			D3D11_BUFFER_DESC tDesc;
			if (GetDescriptionBuffer(tDesc, bStaging))
			{
				ID3D11Buffer *pTexture = nullptr;
				if (FAILED(pDevice->CreateBuffer(&tDesc, pSubResouce.get(),
												 &pTexture)) ||
					pTexture == nullptr)
				{
					return false;
				}
				rpResource = pTexture;
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETexture_Platform::CreateView()
	{
		ID3D11Device *pDevice(IOE::Renderer::IOERPI::GetSingletonPtr()
								  ->GetPlatform()
								  .GetDevice());
		if (pDevice == nullptr)
		{
			return false;
		}
		if (m_pTexture == nullptr)
		{
			return false;
		}
		if (!GetTexture().GetCreateView())
		{
			return true;
		}
		if (GetTexture().GetBufferDescription() &
			EBufferDescription::RenderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC tDesc;
			tDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			if (GetTexture().GetTextureType() == ETextureType::TextureCube ||
				GetTexture().GetTextureType() ==
					ETextureType::TextureCubeArray)
			{
				tDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				tDesc.Texture2DArray.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2DArray.FirstArraySlice =
					GetTexture().GetFirstArrayIndexToMap() * 6 +
					(GetTexture().GetCubeFace() == ECubeFace::All
						 ? 0
						 : static_cast<UINT>(GetTexture().GetCubeFace()));
				tDesc.Texture2DArray.ArraySize =
					GetTexture().GetCubeFace() == ECubeFace::All ? 6 : 1;
			}
			else
			{
				if (GetTexture().IsTextureArray())
				{
					tDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					tDesc.Texture2DArray.MipSlice =
						static_cast<UINT>(GetTexture().GetMipToMap());
					tDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(
						GetTexture().GetFirstArrayIndexToMap());
					tDesc.Texture2DArray.ArraySize =
						static_cast<UINT>(GetTexture().GetArrayCountToMap());
				}
				else
				{
					tDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					tDesc.Texture2D.MipSlice =
						static_cast<UINT>(GetTexture().GetMipToMap());
				}
			}

			ID3D11RenderTargetView **ppRTV =
				reinterpret_cast<ID3D11RenderTargetView **>(&m_pView);
			if (FAILED(pDevice->CreateRenderTargetView(m_pTexture.get(),
													   &tDesc, ppRTV)))
			{
				return false;
			}
			return true;
		}
		else if (GetTexture().GetBufferDescription() &
				 EBufferDescription::DepthStencil)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC tDesc;
			tDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			tDesc.Flags = 0;

			if (GetTexture().GetTextureType() == ETextureType::TextureCube ||
				GetTexture().GetTextureType() ==
					ETextureType::TextureCubeArray)
			{
				tDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				tDesc.Texture2DArray.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2DArray.FirstArraySlice =
					GetTexture().GetCubeFace() == ECubeFace::All
					? 0
					: static_cast<UINT>(GetTexture().GetCubeFace());
				tDesc.Texture2DArray.ArraySize =
					GetTexture().GetCubeFace() == ECubeFace::All ? 6 : 1;
			}
			else
			{
				if (GetTexture().IsTextureArray())
				{
					tDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
					tDesc.Texture2DArray.MipSlice =
						static_cast<UINT>(GetTexture().GetMipToMap());
					tDesc.Texture2DArray.FirstArraySlice = static_cast<UINT>(
						GetTexture().GetFirstArrayIndexToMap());
					tDesc.Texture2DArray.ArraySize =
						static_cast<UINT>(GetTexture().GetArrayCountToMap());
				}
				else
				{
					tDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
					tDesc.Texture2D.MipSlice =
						static_cast<UINT>(GetTexture().GetMipToMap());
				}
			}

			ID3D11DepthStencilView **ppDSV =
				reinterpret_cast<ID3D11DepthStencilView **>(&m_pView);
			if (FAILED(pDevice->CreateDepthStencilView(m_pTexture.get(),
													   &tDesc, ppDSV)))
			{
				return false;
			}
			return true;
		}
		else if (GetTexture().GetBufferDescription() &
				 EBufferDescription::UnorderedAccess)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC tDesc;
			ZeroMemory(&tDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
			tDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			switch (GetTexture().GetTextureType())
			{
			case ETextureType::Texture1D:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1D;
				tDesc.Texture1D.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				break;
			case ETextureType::Texture1DArray:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
				tDesc.Texture1DArray.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture1DArray.FirstArraySlice =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap());
				tDesc.Texture1DArray.ArraySize =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				break;
			case ETextureType::Texture2D:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
				tDesc.Texture2D.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				break;
			case ETextureType::Texture2DArray:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				tDesc.Texture2DArray.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2DArray.FirstArraySlice =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap());
				tDesc.Texture2DArray.ArraySize =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				break;
			case ETextureType::Texture3D:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
				tDesc.Texture3D.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture3D.WSize = -1;
				break;
			case ETextureType::TextureCubeArray:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
				tDesc.Texture2DArray.MipSlice =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2DArray.FirstArraySlice =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap()) *
					6;
				tDesc.Texture2DArray.ArraySize =
					static_cast<UINT>(GetTexture().GetArrayCountToMap()) * 6;
				break;
			case ETextureType::GenericBuffer:
				tDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				tDesc.Buffer.FirstElement =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap());
				tDesc.Buffer.NumElements =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				tDesc.Buffer.Flags = GetTexture().HasImplicitCounter()
					? D3D11_BUFFER_UAV_FLAG_COUNTER
					: 0;
				if (GetTexture().CanAllowRawValues())
				{
					tDesc.Buffer.Flags |= D3D11_BUFFER_UAV_FLAG_RAW;
				}
				break;
			}
			ID3D11UnorderedAccessView **ppUAV =
				reinterpret_cast<ID3D11UnorderedAccessView **>(&m_pView);
			if (FAILED(pDevice->CreateUnorderedAccessView(m_pTexture.get(),
														  &tDesc, ppUAV)))
			{
				return false;
			}
			return true;
		}
		else if (GetTexture().GetBufferDescription() ==
				 EBufferDescription::ShaderResource)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC tDesc;
			tDesc.Format = GetFormat(GetTexture().GetTextureFormat());
			switch (GetTexture().GetTextureType())
			{
			case ETextureType::Texture1D:
				tDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				tDesc.Texture1D.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture1D.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				break;
			case ETextureType::Texture1DArray:
				tDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
				tDesc.Texture1DArray.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture1DArray.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				tDesc.Texture1DArray.FirstArraySlice =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap());
				tDesc.Texture1DArray.ArraySize =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				break;
			case ETextureType::Texture2D:
				tDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				tDesc.Texture2D.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2D.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				break;
			case ETextureType::Texture2DArray:
				tDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				tDesc.Texture2DArray.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture2DArray.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				tDesc.Texture2DArray.FirstArraySlice =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap());
				tDesc.Texture2DArray.ArraySize =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				break;
			case ETextureType::Texture3D:
				tDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				tDesc.Texture3D.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.Texture3D.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				break;
			case ETextureType::TextureCube:
				tDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
				tDesc.TextureCube.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.TextureCube.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				break;
			case ETextureType::TextureCubeArray:
				tDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBEARRAY;
				tDesc.TextureCubeArray.MostDetailedMip =
					static_cast<UINT>(GetTexture().GetMipToMap());
				tDesc.TextureCubeArray.MipLevels = IOE::Maths::Max(
					static_cast<UINT>(GetTexture().GetNumMips()), 1U);
				tDesc.TextureCubeArray.First2DArrayFace =
					static_cast<UINT>(GetTexture().GetFirstArrayIndexToMap()) *
					6;
				tDesc.TextureCubeArray.NumCubes =
					static_cast<UINT>(GetTexture().GetArrayCountToMap());
				break;
			case ETextureType::GenericBuffer:
				if (GetTexture().CanAllowRawValues())
				{
					tDesc.ViewDimension			= D3D_SRV_DIMENSION_BUFFEREX;
					tDesc.BufferEx.FirstElement = static_cast<UINT>(
						GetTexture().GetFirstArrayIndexToMap());
					tDesc.BufferEx.NumElements =
						static_cast<UINT>(GetTexture().GetArrayCountToMap());
					tDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
				}
				else
				{
					tDesc.ViewDimension		  = D3D_SRV_DIMENSION_BUFFER;
					tDesc.Buffer.FirstElement = static_cast<UINT>(
						GetTexture().GetFirstArrayIndexToMap());
					tDesc.Buffer.NumElements =
						static_cast<UINT>(GetTexture().GetArrayCountToMap());
				}
				break;
			}

			ID3D11ShaderResourceView **ppSRV =
				reinterpret_cast<ID3D11ShaderResourceView **>(&m_pView);
			if (FAILED(pDevice->CreateShaderResourceView(m_pTexture.get(),
														 &tDesc, ppSRV)))
			{
				return false;
			}
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::Cleanup()
	{
		ReleaseView();
		ReleaseTexture();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::ReleaseView()
	{
		m_pView.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::ReleaseTexture()
	{
		m_pTexture.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::LoadFromMemory(
		std::uint8_t *pMemoryAddress, std::uint32_t uWidth,
		std::uint32_t uHeight, std::uint32_t uPixelByteWidth /*=4*/)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETexture_Platform::CopyTexture(IOETexture &rhs)
	{
		IOE_ASSERT(rhs.GetPlatform().m_pTexture != nullptr,
				   "Texture not yet set up!");
		m_pTexture = rhs.GetPlatform().m_pTexture;
		if (m_pTexture != nullptr)
		{
			ReleaseView();
			bool bValue(CreateView());
			IOE_ASSERT(bValue, "Failed to create texture");
		}
	}

	//////////////////////////////////////////////////////////////////////////

	/*void SetD3DObjectDebugName(ID3D11DeviceChild *pD3DObject, const char
	*pFmt, ...)
	{
		va_list v;
		char buf[128];

		va_start(v, pFmt);

		_vsnprintf_s<128>(buf, sizeof buf, pFmt, v);
		buf[sizeof buf - 1] = 0;

		va_end(v);

		pD3DObject->SetPrivateData(WKPDID_D3DDebugObjectName,
	static_cast<UINT>(strlen(buf)), buf);
	}*/

	void IOETexture_Platform::SetDebugName(const std::string &szName)
	{
		if (m_pView != nullptr)
		{
			SetD3DObjectDebugName(m_pView.get(), "%s", szName.c_str());
		}
		if (m_pTexture != nullptr)
		{
			SetD3DObjectDebugName(m_pTexture.get(), "%s Texture",
								  szName.c_str());
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE