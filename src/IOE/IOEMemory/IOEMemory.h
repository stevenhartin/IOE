#pragma once

#include <IOE/IOECore/IOEManager.h>
#include <IOE/IOEMemory/IOESmallBlock.h>

namespace IOE
{
namespace Memory
{

	// Class to allocate our own heap which allows us to have tighter control
	// over
	// the
	class HeapManager : public IOE::Core::IOEManager<HeapManager>
	{
	public:
		/************************************************************************
		 * @param const size_t Heap Size
		 *		The size of the heap (in bytes) for the application to manage
		 ************************************************************************/
		HeapManager();
		~HeapManager();

		virtual void OnCreate(void *pConstructionParams = nullptr);

	private:
		size_t m_nHeapSize;

		void *m_pMemAddress;

	public:
		SmallBlock m_tSmallBlock;
	};

} // namespace Memory
} // namespace IOE