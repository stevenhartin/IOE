#include <IOE/IOEMemory/IOEMemoryStatics.h>

namespace IOE
{
namespace Memory
{

	Statics_Platform Statics::m_tPlatform;

	void *Statics::Alloc(size_t nSize, size_t nAlignment, const char *szFile,
						 int nLine)
	{
		return m_tPlatform.Allocate(nSize, nAlignment, szFile, nLine);
	}

	void *Statics::Realloc(void *pAddress, size_t nSize, size_t nAlignment,
						   const char *szFile, int nLine)
	{
		return m_tPlatform.Reallocate(pAddress, nSize, nAlignment, szFile,
									  nLine);
	}

	void Statics::Free(void *pAddress)
	{
		m_tPlatform.Free(pAddress);
	}

} // namespace Memory
} // namespace IOE