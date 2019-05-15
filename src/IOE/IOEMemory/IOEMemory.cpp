#include "IOEMemory.h"

namespace IOE
{
namespace Memory
{

	HeapManager::HeapManager()
		: m_pMemAddress(nullptr)
		, m_nHeapSize(0)
		, IOE::Core::IOEManager<HeapManager>("HeapManager")
		, m_tSmallBlock(512)
	{
	}

	HeapManager::~HeapManager()
	{
	}

	void HeapManager::OnCreate(void *pConstructionParams /*=nullptr*/)
	{
	}

} // namespace Memory
} // namespace IOE