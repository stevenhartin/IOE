#pragma once

#include <IOE/IOEMemory/IOEMemory.h>
#include <IOE/IOEMemory/IOEMemoryBlock.h>

namespace IOE
{
namespace Memory
{

	class BigBlock : public IBlock<BigBlock>
	{
		struct stHeader
		{
#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_MEMORY_DEBUG == IOE_ON)
			char m_szDebugText[32];
#endif
			void *m_pBlockLocation;
			size_t m_nSize;
		};
	};

} // namespace Memory
} // namespace IOE