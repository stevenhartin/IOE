#pragma once

#include <map>
#include <array>
#include <cstdint>
#include <memory>

#include <IOE/IOEMemory/IOEMemory.h>
#include <IOE/IOEMemory/IOEMemoryBlock.h>
#include <IOE/IOEMemory/IOEMemoryChunk.h>

#include <IOE/IOEExceptions/IOEException_Enum.h>
#include <IOE/IOEExceptions/IOEException_Memory.h>

namespace IOE
{
namespace Memory
{

	class ChunkBase;

	class SmallBlock : public IBlock<SmallBlock>
	{
		// Default number of elements to construct within a chunk.
		static const size_t ms_nInitialChunkSize = 64;

	public:
		SmallBlock(size_t nBlockSize)
			: IBlock("SmallBlock", nBlockSize)
		{
		}

		template <typename _T>
		FORCEINLINE _T *Allocate()
		{
			auto &rChunkHeader = FindChunkHeader(sizeof(_T));

			ChunkBase *pChunk(rChunkHeader.GetMemoryAddress());

			if (pChunk == nullptr)
			{
				rChunkHeader.CreateChunk<_T>(ms_nInitialChunkSize);
				pChunk = rChunkHeader.GetMemoryAddress();
			}

			try
			{
				return static_cast<_T *>(pChunk->Allocate());
			}
			catch (IOE::Exceptions::IOEMemoryException &e)
			{
				if (e.GetErrorCode() ==
					IOE::Exceptions::EExceptionID::E_ERROR_OUT_OF_MEMORY)
				{
					// Amount of chunk elements to increase each time the chunk
					// is full
					static const float ms_fReallocationIncrease = 1.5f;

					rChunkHeader.ResizeChunk<_T>(static_cast<std::size_t>(
						(rChunkHeader.GetChunkSize() / sizeof(_T)) *
						ms_fReallocationIncrease));
				}
			}
			return static_cast<_T *>(pChunk->Allocate());
		}

		FORCEINLINE void *
		Allocate(size_t nSize,
				 size_t nAlignment = Statics::GetDefaultAlignment()) OVERRIDE
		{
			THROW_IOE_MEMORY_EXCEPTION(
				"Small block does not support untyped allocation",
				E_ERROR_UNKNOWN_TYPE);
		}

		template <typename _T>
		FORCEINLINE void Release(_T *pAddress)
		{
			Release(pAddress, sizeof(_T));
		}

		FORCEINLINE virtual void Release(void *pAddress,
										 std::size_t nBytes) OVERRIDE
		{
			ChunkBase *pChunk(FindChunkType(nBytes));

			IOE_ASSERT_VARGS(pChunk != nullptr,
							 "Failed to find chunk, are you sure you are "
							 "freeing memory allocated through the small "
							 "block allocator?");
			pChunk->Free(pAddress);
		}

		virtual void Clear() OVERRIDE
		{
		}

	protected:
		FORCEINLINE ChunkHeader &FindChunkHeader(size_t nSize)
		{
#pragma warning(push)
#pragma warning(disable : 4592) // disable warning for runtime calling of
								// constexpr functions
			nSize = NEAREST_MULT(nSize, Statics::GetDefaultAlignment()) /
				Statics::GetDefaultAlignment();
#pragma warning(pop)
			if (nSize < m_arrChunks.max_size())
			{
				return m_arrChunks[nSize];
			}
			THROW_IOE_MEMORY_EXCEPTION(
				"Small block chunks do not go that high!",
				E_ERROR_REQUEST_TOO_LARGE);
		}

		FORCEINLINE ChunkBase *FindChunkType(size_t nSize)
		{
			auto &rChunkHeader = FindChunkHeader(nSize);

			if (rChunkHeader.GetMemoryAddress() != nullptr &&
				rChunkHeader.GetMemoryAddress()->GetSize() == nSize)
			{
				return rChunkHeader.GetMemoryAddress();
			}

			return nullptr;
		}

	private:
		std::array<ChunkHeader, 1024> m_arrChunks;
	};

} // namespace Memory
} // namespace IOE