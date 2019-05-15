#pragma once

#include <vector>
#include <utility>
#include <memory>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOERenderer/IOERPI_Defines.h>
#include RPI_INCLUDE(IOEBuffer_Platform.h)

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::forward;
using std::move;
using std::shared_ptr;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	namespace EBufferDescription
	{
		enum Enum
		{
			Unknown			= 0x0,
			Vertex			= 0x1,
			Index			= 0x2,
			Constant		= 0x4,
			ShaderResource  = 0x8,
			StreamOutput	= 0x10,
			RenderTarget	= 0x20,
			DepthStencil	= 0x40,
			UnorderedAccess = 0x80,
			Decoder			= 0x100,
			VideoEncoder	= 0x200
		};
	}

	//////////////////////////////////////////////////////////////////////////

	enum class EBufferMapType
	{
		ReadOnly,
		WriteOnly,
		ReadWrite,
		WriteDiscard,
		WriteNoOverwrite
	};

	//////////////////////////////////////////////////////////////////////////

	enum class ECPUAccess
	{
		// Data is able to be read from the GPU  Can also be overrwriten on the
		// GPU. This is the default.
		GPUReadWrite,

		// Cannot be accessed at all on the CPU. Constant and must be set up
		// at initialisation time. Essentially the same as GPUReadWrite but
		// is constant.
		GPUReadOnlyConst,

		// Can be accessed and updated by the CPU. To update, you must map the
		// buffer and update. Any changes are sent to the GPU. This is
		// relatively
		// slow.
		GPUReadCPUWrite,

		// The GPU writes back to the CPU every time the buffer is overridden
		// on the
		// GPU. This is the slowest of all.
		GPUWriteBack,
	};

	//////////////////////////////////////////////////////////////////////////

	abstract_class IOEBufferBase
	{
	public:
		virtual std::size_t GetNumElements() const = 0;
		virtual std::size_t GetElementSize() const = 0;
		virtual const void *GetRawData() const = 0;
		virtual void *GetRawData() = 0;
		virtual bool Map(EBufferMapType eMapType, void *&rpData) = 0;
		virtual bool Unmap() = 0;
		virtual EBufferDescription::Enum GetBufferDescription() const = 0;
		virtual ECPUAccess GetCPUAccess() const = 0;
		virtual IOEBuffer_Platform &GetPlatform() = 0;
		virtual const IOEBuffer_Platform &GetPlatform() const = 0;
	};

	//////////////////////////////////////////////////////////////////////////

	template <typename _T>
	class IOEBuffer : public IOEBufferBase
	{
		typedef typename vector<_T> TContainer;
		typedef typename TContainer::size_type TSizeType;

	public:
		//////////////////////////////////////////////////////////////////////////

		IOEBuffer(TSizeType nNumElements)
			: m_arrElementData(nNumElements)
			, m_nNumElements(nNumElements)
			, m_tPlatform(this)
			, m_eCPUAccess(ECPUAccess::GPUReadWrite)
			, m_eBufferDescription(EBufferDescription::Unknown)
		{
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE_DEBUGGABLE void ResizeBuffer(TSizeType nNumElements)
		{
			m_arrElementData.resize(nNumElements);
			m_nNumElements = nNumElements;
		}

		//////////////////////////////////////////////////////////////////////////

		template <class... _TArgs>
		FORCEINLINE_DEBUGGABLE void Emplace(_TArgs &&... Args)
		{
			m_arrElementData.emplace_back(forward<_TArgs>(Args...)...);
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE void Set(TSizeType nIdx, const _T &rValue)
		{
			m_arrElementData.at(nIdx) = rValue;
		}

		//////////////////////////////////////////////////////////////////////////

		template <class... _TArgs>
		FORCEINLINE void SetEmplace(TSizeType nIdx, _TArgs &&... Args)
		{
			m_arrElementData.at(nIdx).Set(Args...);
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE_DEBUGGABLE void Add(const _T &rhs)
		{
			m_arrElementData.push_back(rhs);
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE_DEBUGGABLE void Add(_T &&rhs)
		{
			m_arrElementData.push_back(move(rhs));
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE void Setup(class IOERPI *pPtr)
		{
			m_tPlatform.Setup(pPtr);
		}
		FORCEINLINE void Release()
		{
			m_tPlatform.Release();
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE virtual EBufferDescription::Enum
		GetBufferDescription() const override final
		{
			return m_eBufferDescription;
		}
		FORCEINLINE void
		SetBufferDescription(EBufferDescription::Enum eBufferDescription)
		{
			m_eBufferDescription = eBufferDescription;
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE virtual ECPUAccess GetCPUAccess() const override final
		{
			return m_eCPUAccess;
		}
		FORCEINLINE void SetCPUAccess(ECPUAccess eCPUAccess)
		{
			m_eCPUAccess = eCPUAccess;
		}

		//////////////////////////////////////////////////////////////////////////

		FORCEINLINE virtual std::size_t GetNumElements() const override final
		{
			return m_nNumElements;
		}

		FORCEINLINE virtual std::size_t GetElementSize() const override final
		{
			return sizeof(_T);
		}

		FORCEINLINE virtual const void *GetRawData() const override final
		{
			return m_arrElementData.data();
		}

		FORCEINLINE virtual void *GetRawData() override final
		{
			return m_arrElementData.data();
		}

		FORCEINLINE virtual bool Map(EBufferMapType eMapType,
									 void *&rpData) override final
		{
			return m_tPlatform.Map(eMapType, rpData);
		}

		FORCEINLINE virtual bool Unmap() override final
		{
			return m_tPlatform.Unmap();
		}

		//////////////////////////////////////////////////////////////////////////

		virtual IOEBuffer_Platform &GetPlatform() override final
		{
			return m_tPlatform;
		}
		virtual const IOEBuffer_Platform &GetPlatform() const override final
		{
			return m_tPlatform;
		}

	private:
		IOEBuffer_Platform m_tPlatform;

		TContainer m_arrElementData;
		TSizeType m_nNumElements;

		ECPUAccess m_eCPUAccess;
		EBufferDescription::Enum m_eBufferDescription;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE