#pragma once

#include <utility>
#include <type_traits>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

namespace IOE
{
namespace Core
{
	namespace Algorithm
	{
		template <typename TCharType>
		class TTokeniser
		{
			typedef typename std::basic_string<
				TCharType, std::char_traits<TCharType>,
				std::allocator<TCharType> > TStringClass;

		public:
			struct TTokeniserIterator
			{
			public:
				TTokeniserIterator(const TTokeniser<TCharType> &rTokeniser,
								   size_t nFirstIdx = 0)
					: m_tTokeniser(rTokeniser)
					, m_nBeginIndex(nFirstIdx)
					, m_nNextDelimiter(-1)
				{
					FindNextDelimiter();
				}

				TTokeniserIterator(const TTokeniserIterator &rhs)
					: m_tTokeniser(rhs.m_tTokeniser)
					, m_nBeginIndex(rhs.m_nBeginIndex)
					, m_nNextDelimiter(rhs.m_nNextDelimiter)
				{
				}

				TTokeniserIterator(TTokeniserIterator &&rhs)
					: m_tTokeniser(std::move(rhs.m_tTokeniser))
					, m_nBeginIndex(rhs.m_nBeginIndex)
					, m_nNextDelimiter(rhs.m_nNextDelimiter)
				{
					rhs.m_nBeginIndex	= -1;
					rhs.m_nNextDelimiter = -1;
				}

				bool operator==(const TTokeniserIterator &rhs) const
				{
					return m_nBeginIndex == rhs.m_nBeginIndex;
				}

				bool operator!=(const TTokeniserIterator &rhs) const
				{
					return !(operator==(rhs));
				}

				TTokeniserIterator &operator=(const TTokeniserIterator &rhs)
				{
					m_tTokeniser	 = rhs.m_tTokeniser;
					m_nBeginIndex	= rhs.m_nBeginIndex;
					m_nNextDelimiter = rhs.m_nNextDelimiter;
				}

				TTokeniserIterator &operator=(TTokeniserIterator &&rhs)
				{
					m_tTokeniser		 = std::move(rhs.m_tTokeniser);
					m_nBeginIndex		 = rhs.m_nBeginIndex;
					m_nNextDelimiter	 = rhs.m_nNextDelimiter;
					rhs.m_nBeginIndex	= -1;
					rhs.m_nNextDelimiter = -1;
				}

			public:
				operator TStringClass() const
				{
					return m_tTokeniser.m_szData.substr(m_nBeginIndex,
														m_nNextDelimiter);
				}

				TStringClass operator*() const
				{
					return static_cast<TStringClass>(*this);
				}

				TStringClass operator->() const
				{
					return static_cast<TStringClass>(*this);
				}

			public:
				TTokeniserIterator &operator++()
				{
					Next();
					return *this;
				}

				TTokeniserIterator operator++(int)
				{
					TTokeniserIterator tCopy(*this);
					Next();
					return tCopy;
				}

			private:
				void Next()
				{
					m_nBeginIndex = m_nNextDelimiter;
					FindNextDelimiter();
				}

				void FindNextDelimiter()
				{
					m_nNextDelimiter = m_tTokeniser.m_szData.find(
						m_tTokeniser.m_chDelimiter, m_nBeginIndex);
				}

			private:
				size_t m_nBeginIndex;
				size_t m_nNextDelimiter;
				const TTokeniser<TCharType> &m_tTokeniser;
			};

		public:
			friend TTokeniserIterator;

			TTokeniser(const TStringClass &rInput, TCharType chDelimiter)
				: m_szData(rInput)
				, m_chDelimiter(chDelimiter)
			{
			}

		public:
			TTokeniserIterator begin() const
			{
				return TTokeniserIterator(*this, 0);
			}

			TTokeniserIterator end() const
			{
				return TTokeniserIterator(*this, TStringClass::npos);
			}

		private:
			TStringClass m_szData;
			TCharType m_chDelimiter;
		};
	}
}
}