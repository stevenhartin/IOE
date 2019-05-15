#pragma once

#include <memory>

#include <IOE/IOECore/IOEDefines.h>

//////////////////////////////////////////////////////////////////////////

using std::shared_ptr;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEBufferBase;
	class IOERPI;

	enum class EBufferMapType;

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEBuffer_PlatformBase
	{
	public:
		IOEBuffer_PlatformBase(IOEBufferBase * pBase)
			: m_pBase(pBase)
		{
		}

		virtual void Setup(IOERPI * pPtr) = 0;
		virtual void Release() = 0;

		virtual bool Map(EBufferMapType eMapType, void *&rpData) = 0;
		virtual bool Unmap() = 0;

	protected:
		FORCEINLINE IOEBufferBase *GetBuffer()
		{
			return m_pBase;
		}
		FORCEINLINE const IOEBufferBase *GetBuffer() const
		{
			return m_pBase;
		}

	private:
		IOEBufferBase *m_pBase;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE