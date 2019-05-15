#pragma once

#include <memory>
#include <cstdint>

#include <d3d11.h>

#include <IOE/IOECore/IOEDefines.h>
#include "../IOEBuffer_PlatformBase.h"
#include "IOEComSharedPtr.h"

//////////////////////////////////////////////////////////////////////////

struct ID3D11Buffer;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	enum class ECPUAccess;
	enum class EBufferMapType;

	class IOEBufferBase;

	using std::uint32_t;

	//////////////////////////////////////////////////////////////////////////

	extern D3D11_BIND_FLAG GetBindFlag(uint32_t eBufferType);
	extern D3D11_USAGE GetCPUAccess(ECPUAccess eAccess, bool bStaging = false);
	extern D3D11_MAP GetMapType(EBufferMapType eMapType,
								bool bStaging = false);
	extern D3D11_CPU_ACCESS_FLAG GetTextureAccessType(ECPUAccess eBufferAccess,
													  bool bStaging = false);

	//////////////////////////////////////////////////////////////////////////

	class IOEBuffer_Platform : public IOEBuffer_PlatformBase
	{
	public:
		IOEBuffer_Platform(IOEBufferBase *pBase)
			: IOEBuffer_PlatformBase(pBase)
		{
		}

		virtual void Setup(class IOERPI *pPtr) override final;
		virtual void Release() override final;

		virtual bool Map(EBufferMapType eMapType,
						 void *&rpData) override final;
		virtual bool Unmap() override final;

		FORCEINLINE ID3D11Buffer *GetD3DBuffer()
		{
			return m_pBuffer.get();
		}
		FORCEINLINE const ID3D11Buffer *GetD3DBuffer() const
		{
			return m_pBuffer.get();
		}
		FORCEINLINE const TComSharedPtr<ID3D11Buffer> GetD3DBufferShared() const
		{
			return m_pBuffer;
		}
		FORCEINLINE TComSharedPtr<ID3D11Buffer> GetD3DBufferShared()
		{
			return m_pBuffer;
		}

	private:
		TComSharedPtr<ID3D11Buffer> m_pBuffer;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE