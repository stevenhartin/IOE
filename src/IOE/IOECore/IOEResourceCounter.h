//////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////

#include <cstdint>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	template <typename _T>
	class IOEResourceCounter
	{
	public:
		static const std::uint32_t INVALID_RESOURCE_INDEX = 0;

		static std::uint32_t GetNumResources()
		{
			return ms_uResourceIndex;
		}

	protected:
		IOEResourceCounter()
			: m_uResourceIndex(++ms_uResourceIndex)
		{
		}

	protected:
		inline std::uint32_t GetResourceIndex() const
		{
			return m_uResourceIndex;
		}

	private:
		static std::uint32_t ms_uResourceIndex;
		const std::uint32_t m_uResourceIndex;
	};

	//////////////////////////////////////////////////////////////////////////

	template <typename _T>
	std::uint32_t IOEResourceCounter<_T>::ms_uResourceIndex =
		IOEResourceCounter<_T>::INVALID_RESOURCE_INDEX;

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE

//////////////////////////////////////////////////////////////////////////