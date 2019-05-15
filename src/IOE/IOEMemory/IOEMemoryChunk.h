#pragma once

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOEMemory/IOEMemoryStatics.h>
#include <IOE/IOEExceptions/IOEException_Memory.h>
#include <IOE/IOEExceptions/IOEException_Enum.h>

namespace IOE
{
namespace Memory
{

	abstract_class ChunkBase
	{
	public:
		virtual ~ChunkBase()
		{
		}

		virtual void *Allocate() = 0;
		virtual void *Resize(std::uint32_t uNumElements) = 0;
		virtual void Free(void *pAddress) = 0;

		virtual FORCEINLINE_DEBUGGABLE const size_t GetSize() const
		{
			return 0;
		}
	};

	class ChunkHeader
	{
	public:
		ChunkHeader()
			: m_nChunkSize(0)
			, m_pMemoryAddress(nullptr)
		{
		}

		ChunkHeader(size_t nChunkSize, ChunkBase *pAddress = nullptr)
			: m_nChunkSize(nChunkSize)
			, m_pMemoryAddress(pAddress)
		{
		}

		~ChunkHeader()
		{
			Statics::Free(m_pMemoryAddress);
		}

		template <typename _T>
		FORCEINLINE_DEBUGGABLE ChunkBase *CreateChunk(size_t nNumElements)
		{
			IOE_ASSERT(m_pMemoryAddress == nullptr,
					   "Cannot allocate chunk if one already exists");
			if (m_pMemoryAddress == nullptr)
			{
				m_nChunkSize = nNumElements * sizeof(_T);

				// Allocate memory for the chunk data
				Chunk<_T> *pData(static_cast<Chunk<_T> *>(
					IOE_MEM_ALLOC(sizeof(Chunk<_T>))));

				// Construct chunk which should allocate chunk linked list
				new (pData) Chunk<_T>(nNumElements);

				m_pMemoryAddress = pData;

				return pData;
			}
			return nullptr;
		}

		template <typename _T>
		FORCEINLINE_DEBUGGABLE ChunkBase *
		ResizeChunk(std::uint32_t uNewNumElements)
		{
			IOE_ASSERT(m_pMemoryAddress != nullptr,
					   "Chunk must be allocated before it can be resized");
			m_pMemoryAddress->Resize(uNewNumElements);
			return m_pMemoryAddress;
		}

		FORCEINLINE ChunkBase *GetMemoryAddress()
		{
			return m_pMemoryAddress;
		}
		FORCEINLINE const ChunkBase *GetMemoryAddress() const
		{
			return m_pMemoryAddress;
		}

		FORCEINLINE size_t GetChunkSize() const
		{
			return m_nChunkSize;
		}

	private:
		size_t m_nChunkSize;
		ChunkBase *m_pMemoryAddress;
	};

	template <typename _T>
	class PowerOfTwoSizedChunk
	{
	public:
		PowerOfTwoSizedChunk()
			: m_pNextFree(nullptr)
		{
		}

		operator _T()
		{
			return m_tData;
		}
		PowerOfTwoSizedChunk &operator=(const _T &rhs)
		{
			m_tData = rhs;
			return *this;
		}

	public:
		union
		{
			PowerOfTwoSizedChunk *m_pNextFree;
			_T m_tData;
		};
	};

	template <typename _T>
	class Chunk : public ChunkBase
	{
	public:
		typedef typename IOE::Memory::PowerOfTwoSizedChunk<_T> AlignedChunk;
		typedef AlignedChunk *AlignedChunkPtr;
		static const std::size_t ALLOC_ELEM_SIZE = sizeof(AlignedChunk);

	public:
		Chunk(size_t nBlockSize,
			  size_t nAlignment = Statics::GetDefaultAlignment())
			: m_pHeader(nullptr)
			, m_pFirstFree(nullptr)
			, m_nAlignment(nAlignment)
		{
			m_pHeader = static_cast<AlignedChunkPtr>(IOE_MEM_ALLOC_ALIGNED(
				ALLOC_ELEM_SIZE * nBlockSize, nAlignment));
			new (m_pHeader) AlignedChunk[nBlockSize];
			for (size_t nSize(1); nSize < nBlockSize; ++nSize)
			{
				m_pHeader[nSize - 1].m_pNextFree = &m_pHeader[nSize];
			}
			m_pHeader[nBlockSize - 1].m_pNextFree = nullptr;
			m_pFirstFree						  = m_pHeader;
		}

		~Chunk()
		{
			Statics::Free(m_pHeader);
		}

		_T *const GetHeader()
		{
			return reinterpret_cast<_T *>(m_pHeader);
		}

		const _T *const GetHeader() const
		{
			return reinterpret_cast<const _T *>(m_pHeader);
		}

		virtual void *Allocate() OVERRIDE
		{
			if (m_pFirstFree != nullptr)
			{
				AlignedChunkPtr pFirstFree(m_pFirstFree);
				m_pFirstFree = pFirstFree->m_pNextFree;
				return &pFirstFree->m_tData;
			}
			THROW_IOE_MEMORY_EXCEPTION("Out of memory", E_ERROR_OUT_OF_MEMORY);
		}

		virtual void *Resize(std::uint32_t uNumElements) OVERRIDE
		{
			if (m_pFirstFree != nullptr)
			{
				m_pFirstFree =
					static_cast<AlignedChunkPtr>(IOE_MEM_REALLOC_ALIGNED(
						m_pFirstFree, ALLOC_ELEM_SIZE * uNumElements,
						m_nAlignment));
			}
			return m_pFirstFree;
		}

		virtual void Free(void *pAddress) OVERRIDE
		{
			Free((_T *)(pAddress));
		}

#if (HAS_VARIADIC_TEMPLATES)
		template <class... TArgs>
		_T *Callocate(TArgs &&... Args)
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory)
				_T(std::forward<TArgs>(Args...)...); // call constructor
			return pMemory;
		}
#else
		_T *Callocate()
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory) _T; // call default constructor
			return pMemory;
		}

		template <typename T1>
		_T *Callocate(T1 &&tFirst)
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory) _T(std::forward<_T1>(tFirst));
			return pMemory;
		}

		template <typename T1, typename T2>
		_T *Callocate(T1 &&tFirst, T2 &&tSecond)
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory)
				_T(std::forward<T1>(tFirst), std::forward<T2>(tSecond));
			return pMemory;
		}

		template <typename T1, typename T2, typename T3>
		_T *Callocate(T1 &&tFirst, T2 &&tSecond, T3 &&tThird)
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory)
				_T(std::forward<T1>(tFirst), std::forward<T2>(tSecond),
				   std::forward<T3>(tThird));
			return pMemory;
		}

		template <typename T1, typename T2, typename T3, typename T4>
		_T *Callocate(T1 &&tFirst, T2 &&tSecond, T3 &&tThird, T4 &&tFourth)
		{
			_T *pMemory(static_cast<_T *>(Allocate()));
			new (pMemory)
				_T(std::forward<T1>(tFirst), std::forward<T2>(tSecond),
				   std::forward<T3>(tThird), std::forward<T4>(tFourth));
			return pMemory;
		}
#endif // HAS_VARIADIC_TEMPLATES

		void Free(_T *pAddress)
		{
			if (pAddress != nullptr)
			{
				AlignedChunkPtr pActualAddress(
					reinterpret_cast<AlignedChunkPtr>(pAddress));

				pActualAddress->m_pNextFree = m_pFirstFree;
				m_pFirstFree				= pActualAddress;

				// Manually destruct the type
				(*pAddress).~_T();
			}
		}

		/**
		 * @return size_t The actual size of the chunk (power of two)
		 */
		virtual FORCEINLINE_DEBUGGABLE const size_t GetSize() const OVERRIDE
		{
			return sizeof(_T);
		}

	private:
		std::size_t m_nAlignment;
		AlignedChunkPtr m_pHeader;
		AlignedChunkPtr m_pFirstFree;
	};

} // namespace Memory
} // namespace IOE