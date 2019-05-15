#pragma once

#include "IOEDefines.h"

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEPlatformStatics_PlatformBase
	{
	public:
#if HAS_CONSTEXPR
		static constexpr wchar_t GetPathSplit()
		{
			return L'/';
		}
		static constexpr wchar_t GetIncorrectPathSplit()
		{
			return L'\\';
		}
		static constexpr wchar_t* GetBackDir()
		{
			return L"..";
		}
#else
		// Lets hope these can be fixed up and inserted at compile time without
		// constexpr. Relying on compiler doing some inserting of code rather
		// than
		// simply executing a constant expression.
		FORCEINLINE static wchar_t GetPathSplit()
		{
			return L'/';
		}
		FORCEINLINE static wchar_t GetIncorrectPathSplit()
		{
			return L'\\';
		}
		FORCEINLINE static wchar_t* GetBackDir()
		{
			return L"..";
		}
#endif
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE