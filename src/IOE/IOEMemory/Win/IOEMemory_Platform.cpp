#include "IOEMemory_Platform.h"
#include <stdlib.h>

#include <tbb/scalable_allocator.h>

namespace IOE
{
namespace Memory
{

	void *Statics_Platform::Allocate(size_t nBytes, size_t nAlignment,
									 const char *szFile, int nLine)
	{
		/*#if (IOE_DEBUG_ENABLED && IOE_MEMORY_DEBUG)
			return _aligned_malloc_dbg(nBytes, nAlignment, szFile, nLine);
		#else
			return _aligned_malloc(nBytes, nAlignment);
		#endif*/
		return scalable_aligned_malloc(nBytes, nAlignment);
	}

	void Statics_Platform::Free(void *pAddress)
	{
		/*#if (IOE_DEBUG_ENABLED && IOE_MEMORY_DEBUG)
			_aligned_free_dbg(pAddress);
		#else
			_aligned_free(pAddress);
		#endif*/
		scalable_aligned_free(pAddress);
	}

	void *Statics_Platform::Reallocate(void *pAddress, size_t nBytes,
									   size_t nAlignment, const char *szFile,
									   int nLine)
	{
		/*#if (IOE_DEBUG_ENABLED && IOE_MEMORY_DEBUG)
			return _aligned_realloc_dbg(pAddress, nBytes, nAlignment, szFile,
		nLine);
		#else
			return _aligned_realloc(pAddress, nBytes, nAlignment);
		#endif*/
		return scalable_aligned_realloc(pAddress, nBytes, nAlignment);
	}

} // namespace Memory
} // namespace IOE