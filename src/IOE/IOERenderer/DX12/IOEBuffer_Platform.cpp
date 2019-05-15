#include "IOEBuffer_Platform.h"
#include "../IOEBuffer.h"
#include "../IOERPI.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	D3D11_BIND_FLAG GetBindFlag(uint32_t eBufferType)
	{
		IOE_ASSERT(eBufferType != EBufferDescription::Unknown,
				   "Unknown buffer type!");

		D3D11_BIND_FLAG eBindFlag = (D3D11_BIND_FLAG)0;

		if (eBufferType & EBufferDescription::Vertex)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_VERTEX_BUFFER);
		if (eBufferType & EBufferDescription::Index)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_INDEX_BUFFER);
		if (eBufferType & EBufferDescription::Constant)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_CONSTANT_BUFFER);
		if (eBufferType & EBufferDescription::ShaderResource)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_SHADER_RESOURCE);
		if (eBufferType & EBufferDescription::StreamOutput)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_STREAM_OUTPUT);
		if (eBufferType & EBufferDescription::RenderTarget)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_RENDER_TARGET);
		if (eBufferType & EBufferDescription::DepthStencil)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_DEPTH_STENCIL);
		if (eBufferType & EBufferDescription::UnorderedAccess)
			eBindFlag = (D3D11_BIND_FLAG)(
				eBindFlag + (int32_t)D3D11_BIND_UNORDERED_ACCESS);
		if (eBufferType & EBufferDescription::Decoder)
			eBindFlag =
				(D3D11_BIND_FLAG)(eBindFlag + (int32_t)D3D11_BIND_DECODER);
		if (eBufferType & EBufferDescription::VideoEncoder)
			eBindFlag = (D3D11_BIND_FLAG)(eBindFlag +
										  (int32_t)D3D11_BIND_VIDEO_ENCODER);

		return eBindFlag;
	}

	//////////////////////////////////////////////////////////////////////////

	D3D12_HEAP_TYPE GetHeapType(ECPUAccess eAccess)
	{
		switch (eAccess)
		{
		case ECPUAccess::GPUReadWrite:
			return D3D12_HEAP_TYPE_DEFAULT;
		case ECPUAccess::GPUReadOnlyConst:
			return D3D12_HEAP_TYPE_DEFAULT;
		case ECPUAccess::GPUReadCPUWrite:
			return D3D12_HEAP_TYPE_UPLOAD;
		case ECPUAccess::GPUWriteBack:
			return D3D12_HEAP_TYPE_READBACK;
		default:
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	D3D12_CPU_PAGE_PROPERTY GetCPUAccess(ECPUAccess eAccess, bool bStaging /*=false*/)
	{
		switch (eAccess)
		{
		case ECPUAccess::GPUReadWrite:
			return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		case ECPUAccess::GPUReadOnlyConst:
			return D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE;
		case ECPUAccess::GPUReadCPUWrite:
			return D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
		case ECPUAccess::GPUWriteBack:
			return D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		default:
			return D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_MAP GetMapType(EBufferMapType eMapType, bool bStaging /*=false*/)
	{
		switch (eMapType)
		{
		case EBufferMapType::ReadOnly:
			return D3D11_MAP_READ;
		case EBufferMapType::WriteOnly:
			return D3D11_MAP_WRITE;
		case EBufferMapType::ReadWrite:
			return D3D11_MAP_READ_WRITE;
		case EBufferMapType::WriteDiscard:
			return D3D11_MAP_WRITE_DISCARD;
		case EBufferMapType::WriteNoOverwrite:
			return D3D11_MAP_WRITE_NO_OVERWRITE;
		}
		return D3D11_MAP_READ;
	}

	//////////////////////////////////////////////////////////////////////////

	D3D11_CPU_ACCESS_FLAG GetTextureAccessType(ECPUAccess eBufferAccess,
											   bool bStaging /*=false*/)
	{
		switch (eBufferAccess)
		{
		case ECPUAccess::GPUReadWrite:
			return (D3D11_CPU_ACCESS_FLAG)0;
		case ECPUAccess::GPUReadOnlyConst:
			return (D3D11_CPU_ACCESS_FLAG)0;
		case ECPUAccess::GPUReadCPUWrite:
			return D3D11_CPU_ACCESS_WRITE;
		case ECPUAccess::GPUWriteBack:
			return bStaging ? D3D11_CPU_ACCESS_READ : (D3D11_CPU_ACCESS_FLAG)0;
		}
		return (D3D11_CPU_ACCESS_FLAG)0;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEBuffer_Platform::Setup(IOERPI *pRPI)
	{
		ID3D12Device *pDevice(pRPI->GetPlatform().GetDevice());

		D3D12_HEAP_PROPERTIES tHeapProperties;
		D3D12_HEAP_FLAGS tHeapFlags;
		D3D12_RESOURCE_DESC tResourceDesc;
		D3D12_RESOURCE_STATES tResourceState;
		D3D12_CLEAR_VALUE tClearValue; // @optional
		REFIID tResourceID;
		void *pResource; // @optional out

		tHeapProperties.Type = GetHeapType(GetBuffer()->GetCPUAccess());
		tHeapProperties.CPUPageProperty = GetCPUAccess(GetBuffer()->GetCPUAccess());
		tHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		tHeapProperties.CreationNodeMask = 0;
		tHeapProperties.VisibleNodeMask = 0;

		tHeapFlags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;

		tResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		tResourceDesc.Alignment = 0;
		tResourceDesc.Width = GetBuffer()->GetElementSize() * GetBuffer()->GetNumElements();
		tResourceDesc.Height = 1;
		tResourceDesc.DepthOrArraySize = 1;
		tResourceDesc.MipLevels = 1;
		tResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		tResourceDesc.SampleDesc.Count = 1;
		tResourceDesc.SampleDesc.Quality = 0;
		tResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		tResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		tResourceState = /*D3D12_RESOURCE_FLAG_*/0;

		tClearValue.Format = GetBuffer()->GetBufferDescription();
		tClearValue.Color[0] = 0.0f;
		tClearValue.Color[1] = 0.0f;
		tClearValue.Color[2] = 0.0f;
		tClearValue.Color[3] = 0.0f;

		/*D3D11_BUFFER_DESC tBufferDesc;
		tBufferDesc.ByteWidth = static_cast<UINT>(
			GetBuffer()->GetElementSize() * GetBuffer()->GetNumElements());
		tBufferDesc.Usage = static_cast<D3D11_USAGE>(
			GetCPUAccess(GetBuffer()->GetCPUAccess()));
		tBufferDesc.BindFlags =
			GetBindFlag(GetBuffer()->GetBufferDescription());
		tBufferDesc.MiscFlags = 0;
		tBufferDesc.CPUAccessFlags =
			GetTextureAccessType(GetBuffer()->GetCPUAccess());
		tBufferDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA tSetupData;
		tSetupData.pSysMem			= GetBuffer()->GetRawData();
		tSetupData.SysMemPitch		= 0;
		tSetupData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&tBufferDesc, &tSetupData, &m_pBuffer);*/
		pDevice->CreateCommittedResource(
			&tHeapProperties, tHeapFlags, &tResourceDesc, tResourceState,
			&tClearValue, tResourceID, &pResource);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEBuffer_Platform::Release()
	{
		m_pBuffer.reset();
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEBuffer_Platform::Map(EBufferMapType eMapType, void *&rpData)
	{
		IOE_ASSERT(GetBuffer()->GetCPUAccess() != ECPUAccess::GPUWriteBack,
				   "Staging buffers not supported!");

		ID3D11DeviceContext *pDeviceContext(
			IOE::Renderer::IOERPI::GetSingletonPtr()
				->GetPlatform()
				.GetDeviceContext());

		ID3D11Resource *pTexture(m_pBuffer.get());

		D3D11_MAPPED_SUBRESOURCE tMappedSubresource;
		if (SUCCEEDED(pDeviceContext->Map(pTexture, 0, GetMapType(eMapType), 0,
										  &tMappedSubresource)))
		{
			rpData = tMappedSubresource.pData;
			return true;
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOEBuffer_Platform::Unmap()
	{
		IOE_ASSERT(GetBuffer()->GetCPUAccess() != ECPUAccess::GPUWriteBack,
				   "Staging buffers not supported!");

		ID3D11DeviceContext *pDeviceContext(
			IOE::Renderer::IOERPI::GetSingletonPtr()
				->GetPlatform()
				.GetDeviceContext());

		// TODO: Support staging
		ID3D11Resource *pTexture(m_pBuffer.get());

		pDeviceContext->Unmap(pTexture, 0);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE