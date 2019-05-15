#pragma once

#include <IOE/IOECore/IOEDefines.h>

#include PLATFORM_INCLUDE(IOEMemory_Platform.h)

#define IOE_MEM_ALLOC(nSize)                                                  \
	::IOE::Memory::Statics::Alloc(                                            \
		nSize, ::IOE::Memory::Statics::GetDefaultAlignment(), __FILE__,       \
		__LINE__)
#define IOE_MEM_ALLOC_ALIGNED(nSize, nAlignment)                              \
	::IOE::Memory::Statics::Alloc(nSize, nAlignment, __FILE__, __LINE__)

#define IOE_MEM_REALLOC(pAddr, nSize)                                         \
	::IOE::Memory::Statics::Realloc(                                          \
		pAddr, nSize, ::IOE::Memory::Statics::GetDefaultAlignment(),          \
		__FILE__, __LINE__)
#define IOE_MEM_REALLOC_ALIGNED(pAddr, nSize, nAlignment)                     \
	::IOE::Memory::Statics::Realloc(pAddr, nSize, nAlignment, __FILE__,       \
									__LINE__)

#define IOE_MEM_FREE(pAddress) ::IOE::Memory::Statics::Free(pAddress)

namespace IOE
{
namespace Memory
{

#define DEFAULT_MEMORY_ALIGNMENT 8

	class Statics
	{
	public:
		static const size_t GetDefaultAlignment()
		{
			return DEFAULT_MEMORY_ALIGNMENT;
		}
		static void *Alloc(size_t nSize, size_t nAlignment, const char *szFile,
						   int nLine);
		static void *Realloc(void *pAddress, size_t nSize, size_t nAlignment,
							 const char *szFile, int nLine);
		static void Free(void *pAddress);

	private:
		static Statics_Platform m_tPlatform;
	};
}
}