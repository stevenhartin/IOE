#pragma once

#include <IOE/IOEMemory/IOEMemoryStaticsBase.h>

namespace IOE
{
namespace Memory
{
	class Statics_Platform : public Statics_Base
	{
	public:
		virtual void *Allocate(size_t nBytes, size_t nAlignment,
							   const char *szFile, int nLine) OVERRIDE;
		virtual void *Reallocate(void *pAddress, size_t nBytes,
								 size_t nAlignment, const char *szFile,
								 int nLine) OVERRIDE;
		virtual void Free(void *pAddress) OVERRIDE;
	};
} // namesapce Memory
} // namespace IOE