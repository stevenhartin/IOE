#pragma once

#include <iterator>
#include <algorithm>

#include "IOETokeniser.h"

namespace IOE
{
namespace Core
{
	namespace Algorithm
	{
		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType &
		TrimLeft(TStringType &szInput)
		{
			std::locale loc;
			szInput.erase(
				szInput.begin(),
				std::find_if(
					szInput.begin(), szInput.end(),
					[&loc](const TStringType::value_type &chInput) -> bool {
						return !std::isspace<typename TStringType::value_type>(
							chInput, loc);
					}));
			return szInput;
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType &
		TrimRight(TStringType &szInput)
		{
			std::locale loc;
			szInput.erase(
				std::find_if(
					szInput.rbegin(), szInput.rend(),
					[&loc](const TStringType::value_type &chInput) -> bool {
						return !std::isspace<typename TStringType::value_type>(
							chInput, loc);
					})
					.base(),
				szInput.end());
			return szInput;
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType &Trim(TStringType &szInput)
		{
			return TrimLeft(TrimRight(szInput));
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType &
		ToUpper(TStringType &szInput)
		{
			std::transform(std::begin(szInput), std::end(szInput),
						   std::begin(szInput), ::toupper);
			return szInput;
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType
		ToUpper(const TStringType &szInput)
		{
			TStringType tCopy;
			std::transform(std::begin(szInput), std::end(szInput),
						   std::back_inserter(tCopy), ::toupper);
			return tCopy;
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType &
		ToLower(TStringType &szInput)
		{
			std::transform(std::begin(szInput), std::end(szInput),
						   std::begin(szInput), ::tolower);
			return szInput;
		}

		template <typename TStringType>
		static FORCEINLINE_DEBUGGABLE TStringType
		ToLower(const TStringType &szInput)
		{
			TStringType tCopy;
			std::transform(std::begin(szInput), std::end(szInput),
						   std::back_inserter(tCopy), ::tolower);
			return tCopy;
		}

		FORCEINLINE_DEBUGGABLE std::wstring
		ConvertNarrowToWide(const std::string &szInput)
		{
			return std::wstring(szInput.begin(), szInput.end());
		}

		FORCEINLINE_DEBUGGABLE std::string
		ConvertWideToNarrow(const std::wstring &szInput)
		{
			return std::string(szInput.begin(), szInput.end());
		}

		template <class TStringType, class TIteratorType>
		FORCEINLINE_DEBUGGABLE TStringType Join(TIteratorType begin,
												const TIteratorType end,
												const TStringType &sep)
		{
			std::basic_ostringstream<TStringType::value_type,
									 TStringType::traits_type,
									 TStringType::allocator_type> tResult;

			if (begin != end)
			{
				tResult << *begin++;
			}

			while (begin != end)
			{
				tResult << sep;
				tResult << *begin++;
			}
			return tResult.str();
		}

		template <class... TArgs>
		struct JoinPathRecursion;

		template <class TFirst, class... TArgs>
		struct JoinPathRecursion<TFirst, TArgs...>
		{
			static std::wstring _(const std::wstring &szFirst,
								  TArgs &&... Args)
			{
				if (sizeof...(Args) == 0)
				{
					return szFirst;
				}
				std::wstring szEnd(JoinPathRecursion<TArgs...>::_(
					Args...)); // Pull first one variadic list
				return szFirst +
					std::wstring(1, ::IOE::Core::IOEPlatformStatics_Platform::
										GetPathSplit()) +
					szEnd;
			}
		};

		template <class TFirst>
		struct JoinPathRecursion<TFirst>
		{
			static std::wstring _(const TFirst &lhs)
			{
				return lhs;
			}
		};

		template <class... TArgs>
		FORCEINLINE_DEBUGGABLE std::wstring JoinPath(TArgs &&... Args)
		{
			static_assert(sizeof...(TArgs) >= 2,
						  "Can only join paths of 2 elements!");
			return JoinPathRecursion<TArgs...>::_(Args...);
		}

		template <class TStringType,
				  template <class, class> class TContainer = std::vector>
		FORCEINLINE_DEBUGGABLE
			TContainer<TStringType, std::allocator<TStringType> >
			Split(const TStringType &szInput, const TStringType &szDelimiter,
				  const bool bKeepEmpty = true)
		{
			TContainer<TStringType, std::allocator<TStringType> > arrResult;

			auto pStartIt = std::begin(szInput);
			decltype(pStartIt) pEndIt;
			while (true)
			{
				pEndIt = std::search(pStartIt, std::end(szInput),
									 std::begin(szDelimiter),
									 std::end(szDelimiter));
				TStringType szSubStr(pStartIt, pEndIt);
				if (bKeepEmpty || !szSubStr.empty())
				{
					arrResult.push_back(szSubStr);
				}
				if (pEndIt == std::end(szInput))
				{
					break;
				}
				pStartIt = pEndIt + szDelimiter.size();
			}
			return arrResult;
		}

		FORCEINLINE_DEBUGGABLE std::wstring
		GetBasePath(const std::wstring &szInput, bool bOnlyIfFile = true)
		{
			std::size_t uDotPos(szInput.find_last_of(L'.'));
			std::size_t uLastPathChar(szInput.find_last_of(
				::IOE::Core::IOEPlatformStatics_Platform::GetPathSplit()));
			if (bOnlyIfFile)
			{
				if (uDotPos == std::wstring::npos)
				{
					// No dot in file path, can't be a file (assuming the file
					// has an extension)
					return szInput;
				}
				if (uLastPathChar == std::wstring::npos ||
					uDotPos < uLastPathChar)
				{
					// Dot is not in the last directory, so there is no file at
					// the end
					return szInput;
				}
			}
			return szInput.substr(0, uLastPathChar);
		}

		FORCEINLINE_DEBUGGABLE std::wstring
		GetFilename(const std::wstring &szInput,
					bool bIncludeExtension = false)
		{
			std::size_t uDotPos(szInput.find_last_of(L'.'));
			std::size_t uLastPathChar(szInput.find_last_of(
				::IOE::Core::IOEPlatformStatics_Platform::GetPathSplit()));

			if (uLastPathChar == std::wstring::npos)
			{
				uLastPathChar = 0;
			}
			else
			{
				++uLastPathChar;
			}

			std::size_t nEndPos = szInput.size();
			if (!bIncludeExtension && uDotPos != std::wstring::npos)
			{
				nEndPos = uDotPos;
			}
			return szInput.substr(uLastPathChar, nEndPos - uLastPathChar);
		}

		FORCEINLINE_DEBUGGABLE std::wstring
		GetExtension(const std::wstring &szInput)
		{
			std::size_t uDotPos(szInput.find_last_of(L'.'));
			std::size_t uLastPathChar(szInput.find_last_of(
				::IOE::Core::IOEPlatformStatics_Platform::GetPathSplit()));

			if (uDotPos == std::wstring::npos)
			{
				// No dot in file path, can't be a file (assuming the file has
				// an extension)
				return L"";
			}
			if (uLastPathChar == std::wstring::npos || uDotPos < uLastPathChar)
			{
				// Dot is not in the last directory, so there is no file at the
				// end
				return szInput;
			}

			return szInput.substr(uDotPos + 1);
		}

		FORCEINLINE_DEBUGGABLE void FixupPath(std::wstring &szInput)
		{
			std::size_t nLastFoundIdx(0);
			std::size_t nNextFoundIdx(0);
			while ((nNextFoundIdx = szInput.find_first_of(
						::IOE::Core::IOEPlatformStatics_Platform::
							GetIncorrectPathSplit(),
						nLastFoundIdx)) != std::wstring::npos)
			{
				szInput[nNextFoundIdx] =
					::IOE::Core::IOEPlatformStatics_Platform::GetPathSplit();
				nLastFoundIdx = nNextFoundIdx;
			}
			// Resolve to absolute path, remove all back-directory characters
			if (szInput.find(IOEPlatformStatics_Platform::GetBackDir()) !=
				std::wstring::npos)
			{
				std::vector<std::wstring> Tokens(
					Split(szInput,
						  std::wstring(
							  1, IOEPlatformStatics_Platform::GetPathSplit()),
						  false));

				std::size_t uNumSections(Tokens.size());
				for (std::size_t uIdx(0); uIdx < uNumSections - 1;)
				{
					if (Tokens.at(uIdx) ==
						::IOE::Core::IOEPlatformStatics_Platform::GetBackDir())
					{
						// If we landed on a backdir then this is applied to
						// the previous directory
						auto pFirst  = std::begin(Tokens);
						auto pSecond = pFirst;
						std::advance(pFirst, uIdx - 1);
						std::advance(pSecond, uIdx +
										 1); // 1 past the position we erase to
						Tokens.erase(pFirst, pSecond);
						uNumSections -= 2;
						uIdx -= 1;
					}
					if (Tokens.at(uIdx + 1) ==
						::IOE::Core::IOEPlatformStatics_Platform::GetBackDir())
					{
						// If the next directory is a backdir, then this is the
						// directory which is removed.
						auto pFirst  = std::begin(Tokens);
						auto pSecond = pFirst;
						std::advance(pFirst, uIdx);
						std::advance(pSecond, uIdx +
										 2); // 1 past the position we erase to
						Tokens.erase(pFirst, pSecond);
						uNumSections -= 2;
					}
					else
					{
						++uIdx;
					}
				}

				// Join up the paths again.
				szInput = Join<std::wstring>(
					std::begin(Tokens), std::end(Tokens),
					std::wstring(1, ::IOE::Core::IOEPlatformStatics_Platform::
										GetPathSplit()));
			}
		}
	}
}
}