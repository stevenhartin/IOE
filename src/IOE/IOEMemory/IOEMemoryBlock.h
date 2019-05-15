#pragma once

#include <IOE/IOECore/IOEDefines.h>

namespace IOE
{
namespace Memory
{

	class ChunkBase;

	template <class _TDerived>
	interface_class IBlock
	{
	public:
		IBlock(const char *szDebugText, size_t nSize)
		{
		}
		virtual ~IBlock()
		{
		}

	public:
		/************************************************************************
		 * Allocates memory from the memory block.
		 *
		 * @tparam _T The type to allocate
		 * @param size_t nAlignment The boundary to align on
		 * @return _T* The address of the newed memory address
		 ************************************************************************/
		template <typename _T>
		_T *Allocate(size_t nAlignment = 16)
		{
			return static_cast<_TDerived *>(this)->(Allocate<_T>(nAlignment));
		}

		/************************************************************************
		 * Release the memory at the address
		 *
		 * @tparam _T The type of memory to release
		 * @param void* pAddress The address to release
		 ************************************************************************/
		template <typename _T>
		void Release(_T * pAddress)
		{
			Release((void *)pAddress, sizeof(_T));
		}

		/************************************************************************
		* Clear all memory held by this block
		************************************************************************/
		virtual void Clear() = 0;

	protected:
		/************************************************************************
		 * Allocates memory from the memory block.
		 *
		 * @param size_t nSize The amount of bytes to allocate
		 * @param size_t nAlignment The boundary to align on
		 ************************************************************************/
		virtual void *Allocate(size_t nSize, size_t nAlignment = 16) = 0;

		/************************************************************************
		 * Release the memory at the address
		 *
		 * @param void* pAddress The address to release
		 * @param size_t nSize The size of the memory address to release
		 ************************************************************************/
		virtual void Release(void *pAddress, size_t nSize) = 0;
	};

} // namespace Memory
} // namespace IOE