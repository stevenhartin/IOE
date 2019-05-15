#pragma once

// For _assert
#include <type_traits>
#include <limits>
#include <cstdint>
#include <crtdbg.h>

// Synonyms for class to help describe the class better - helps readability
#define interface_class class
#define abstract_class class

#define IOE_OFF 0
#define IOE_ON 1

#if defined(IOE_DEBUG)
#include "IOEDefines_Debug.h"
#elif defined(IOE_RELEASE)
#include "IOEDefines_Release.h"
#elif defined(IOE_FINAL)
#include "IOEDefines_Final.h"
#endif

// Pre-method declarative syntax
#define FORCEINLINE __forceinline
#define INLINE inline

#if defined(IOE_DEBUG)
#define FORCEINLINE_DEBUGGABLE inline
#elif defined(IOE_RELEASE) // IOE_RELEASE
#define FORCEINLINE_DEBUGGABLE inline
#else // IOE_FINAL
#define FORCEINLINE_DEBUGGABLE __forceinline
#endif

// Post-method virtual modifiers
#define FINAL final
#define OVERRIDE override

// Convert the value of any define to a string representation; i.e.
// #define foo 1
// STRINGIFY_EXPAND(foo) -> STRINGIFY(1) -> "1"
// STRINGIFY(foo) -> "foo"
#define STRINGIFY(str) #str
#define STRINGIFY_EXPAND(macro) STRINGIFY(macro)

// Obtain the nearest next power of two
template <typename T,
		  typename = typename std::enable_if<std::is_integral<T>::value>::type,
		  typename = typename std::enable_if<std::is_unsigned<T>::value>::type,
		  typename T2 = typename std::make_unsigned<T>::type>
constexpr T2 NEAREST_POWER_OF_TWO(T value, std::uint32_t curb = 1)
{
	return std::numeric_limits<T2>::digits <= curb
		? value
		: NEAREST_POWER_OF_TWO(((static_cast<T2>(value) - 1) |
								(((static_cast<T2>(value) - 1)) >> curb)) +
								   1,
							   curb << 1);
}

template <
	typename T, typename T2,
	typename = typename std::enable_if<std::is_integral<T>::value>::type,
	typename = typename std::enable_if<std::is_integral<T2>::value>::type>
constexpr T NEAREST_MULT(T num, T2 mult)
{
	return ((num - 1) + (mult - 1) - ((num - 1) % mult) + 1);
}

template <typename T,
		  typename = typename std::enable_if<std::is_integral<T>::value>::type>
constexpr T MOST_SIGNIFICANT_BIT(
	T num, std::int32_t bit = std::numeric_limits<T>::digits - 1)
{
	return ((num & 1 << bit)
				? bit
				: ((bit <= 0) ? 0 : MOST_SIGNIFICANT_BIT(num, bit - 1)));
}

template <
	typename T,
	typename = typename std::enable_if<std::is_integral<T>::value>::type>
constexpr T LEAST_SIGNIFICANT_BIT(T num, T bit = 0)
{
	return (
		(bit >= std::numeric_limits<T>::digits)
			? std::numeric_limits<T>::digits
			: (num & 1 << bit ? bit : LEAST_SIGNIFICANT_BIT(num, bit + 1)));
}

static_assert(MOST_SIGNIFICANT_BIT(0x1000) == 12, "MSB failure");
static_assert(LEAST_SIGNIFICANT_BIT(0x1002) == 1, "MSB failure");

template <typename T, typename T2>
T CopyAndMovePtr(T2 *&pPtr)
{
	// Obtain pointer to the original pointer value
	T *pValue(reinterpret_cast<T *>(pPtr));
	// Move on the pointer by sizeof(T) bytes
	pPtr = reinterpret_cast<T2 *>(reinterpret_cast<std::uint8_t *>(pPtr) +
								  sizeof(T));
	// Return a copy of the value at the address
	return *pValue;
}

template <typename T>
void CopyAndMovePtr(void *pBuffer, T *&pInput, std::size_t nBytes)
{
	// Copy into destination
	memcpy(pBuffer, pInput, nBytes);

	// Move on the pointer by number of bytes
	pPtr =
		reinterpret_cast<T *>(reinterpret_cast<std::uint8_t *>(pPtr) + nBytes);
}

template <typename T, typename T2>
T *GetPointerAndMovePtr(T2 *&pPtr)
{
	// Obtain pointer to the original pointer value
	T *pValue(reinterpret_cast<T *>(pPtr));
	// Move on the pointer by sizeof(T) bytes
	pPtr = reinterpret_cast<T2 *>(reinterpret_cast<std::uint8_t *>(pPtr) +
								  sizeof(T));
	// Return a pointer to the value at the address
	return pValue;
}

template <typename T>
void *GetPointerAndMovePtr(T *&pPtr, std::size_t nBytes)
{
	// Take a copy of the original
	void *pOrigin(pPtr);

	// Move on the pointer by number of bytes
	pPtr =
		reinterpret_cast<T *>(reinterpret_cast<std::uint8_t *>(pPtr) + nBytes);

	// Return a pointer to the value at the address
	return pOrigin;
}

#define RPI_INCLUDE(file_name) STRINGIFY_EXPAND(./IOE_RPI/file_name)
#define PLATFORM_INCLUDE(file_name)                                           \
	STRINGIFY_EXPAND(./IOE_PLATFORM/file_name)
#define EXTERNAL_PLATFORM_INCLUDE(module, file_name)                          \
	STRINGIFY_EXPAND(IOE/module/IOE_PLATFORM/file_name)

// Code taken from here:
// http://www.codeproject.com/Articles/10022/DebugBreak-and-ASSERTs-that-work-always-anywhere
#include <Windows.h> // TODO: Move this to platform specific code
inline void __stdcall PreDebugBreakAnyway()
{
	if (IsDebuggerPresent())
	{
		// We're running under the debugger.
		// There's no need to call the inner DebugBreak
		// placed in the two __try/__catch blocks below,
		// because the outer DebugBreak will
		// force a first-chance exception handled in the debugger.
		return;
	}

	__try
	{
		__try
		{
			DebugBreak();
		}
		__except (UnhandledExceptionFilter(GetExceptionInformation()))
		{
			// You can place the ExitProcess here to emulate work
			// of the __except block from BaseStartProcess
			ExitProcess(0);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// We'll get here if the user has pushed Cancel (Debug).
		// The debugger is already attached to our process.
		// Return to let the outer DebugBreak be called.
	}
}

#define DebugBreakAnyway()                                                    \
	PreDebugBreakAnyway();                                                    \
	DebugBreak();

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_ASSERT_ENABLED == IOE_ON &&           \
	 defined(_DEBUG))

#define IOE_ASSERT(condition, message)                                        \
	do                                                                        \
	{                                                                         \
		if ((!(condition)) &&                                                 \
			(1 == _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__), __LINE__,  \
								 NULL, _CRT_WIDE(message))))                  \
		{                                                                     \
			DebugBreakAnyway();                                               \
		}                                                                     \
	} while (0);

#define IOE_ASSERT_VARGS(condition, message, ...)                             \
	{                                                                         \
		wchar_t buffer[256];                                                  \
		swprintf_s(buffer, 256, _CRT_WIDE(message), __VA_ARGS__);             \
		do                                                                    \
		{                                                                     \
			if (!(condition) &&                                               \
				(1 == _CrtDbgReportW(_CRT_ASSERT, _CRT_WIDE(__FILE__),        \
									 __LINE__, NULL, buffer)))                \
			{                                                                 \
				DebugBreakAnyway();                                           \
			}                                                                 \
		} while (0);                                                          \
	}

#else // IOE_DEBUG_ENABLED == IOE_ON && IOE_ASSERT == IOE_ON

#define IOE_ASSERT(condition, message) ((void)0);

#define IOE_ASSERT_VARGS(condition, message, ...) ((void)0);

#endif // IOE_DEBUG_ENABLED == IOE_ON && IOE_ASSERT == IOE_ON

#if (IOE_DEBUG_ENABLED == IOE_ON && IOE_RPI_PROFILE_MARKERS == IOE_ON)

#define START_PROFILE_MARKER(name, colour)                                    \
	::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(             \
		name##" "##STRINGIFY_EXPAND(__FILE__)##" @ "##STRINGIFY_EXPAND(       \
			__LINE__),                                                        \
		::IOE::Renderer::EProfileMarkerType::Start, colour)
#define END_PROFILE_MARKER()                                                  \
	::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(             \
		STRINGIFY_EXPAND(__FILE__)##" @ "##STRINGIFY_EXPAND(__LINE__),        \
		::IOE::Renderer::EProfileMarkerType::End,                             \
		::IOE::Maths::ColourList::White)
#define SET_PROFILE_MARKER(name, colour)                                      \
	::IOE::Renderer::IOERPI::GetSingletonPtr()->SetProfileMarker(             \
		name##" "##STRINGIFY_EXPAND(__FILE__)##" @ "##STRINGIFY_EXPAND(       \
			__LINE__),                                                        \
		::IOE::Renderer::EProfileMarkerType::Single, colour)

#else

#define START_PROFILE_MARKER(name, colour) ((void)0)
#define END_PROFILE_MARKER() ((void)0)
#define SET_PROFILE_MARKER(name, colour) ((void)0)

#endif