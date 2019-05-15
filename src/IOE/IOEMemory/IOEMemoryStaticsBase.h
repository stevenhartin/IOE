#pragma once

#include <IOE/IOECore/IOEDefines.h>

namespace IOE
{
namespace Memory
{

	abstract_class Statics_Base
	{
	public:
		virtual ~Statics_Base()
		{
		}

		virtual void *Allocate(size_t nBytes, size_t nAlignment,
							   const char *szFile, int nLine) = 0;
		virtual void *Reallocate(void *pAddress, size_t nBytes,
								 size_t nAlignment, const char *szFile,
								 int nLine) = 0;
		virtual void Free(void *pAddress) = 0;
	};
}
}