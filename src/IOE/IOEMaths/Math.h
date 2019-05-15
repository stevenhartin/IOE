#pragma once

#include <IOE/IOECore/IOEDefines.h>

#define IOE_EPSILON (1.0e-7)
#define IOE_EPSILON_F (1.0e-7f)
#define IOE_TINY (1.0e-20)
#define IOE_TINY_F (1.0e-20f)
#define IOE_PI (3.14159265358979323846)
#define IOE_PI_F (3.14159265358979323846f)
#define IOE_TWO_PI (6.28318530717958647692)
#define IOE_TWO_PI_F (6.28318530717958647692f)
#define IOE_RAD_TO_DEG (180.0 / IOE_PI)
#define IOE_RAD_TO_DEG_F (180.0f / IOE_PI_F)
#define IOE_DEG_TO_RAD (1.0 / (IOE_RAD_TO_DEG))
#define IOE_DEG_TO_RAD_F (1.0f / (IOE_RAD_TO_DEG_F))

namespace IOE
{
namespace Maths
{

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Abs(_T tValue)
	{
		if (tValue < (_T)0)
			return (_T)-tValue;
		return tValue;
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Sign(_T tValue)
	{
		if (tValue < (_T)0)
			return (_T)-1;
		else if (tValue > (_T)0)
			return (_T)1;
		return (_T)0;
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Min(_T tValue, _T tMin)
	{
		if (tValue < tMin)
		{
			return tValue;
		}
		return tMin;
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Max(_T tValue, _T tMax)
	{
		if (tValue > tMax)
		{
			return tValue;
		}
		return tMax;
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Clamp(_T tValue, _T tMin, _T tMax)
	{
		return Min(Max(tValue, tMin), tMax);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Sin(_T tValue)
	{
		return ::sin(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T ASin(_T tValue)
	{
		return ::asin(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Cos(_T tValue)
	{
		return ::cos(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T ACos(_T tValue)
	{
		return ::acos(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T Tan(_T tValue)
	{
		return ::tan(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T ATan(_T tValue)
	{
		return ::atan(tValue);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T ATan2(_T tX, _T tY)
	{
		return ::atan2(tX, tY);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T DegreesToRadians(_T tValue)
	{
		return tValue * static_cast<_T>(IOE_DEG_TO_RAD);
	}

	template <typename _T>
	FORCEINLINE_DEBUGGABLE _T RadiansToDegrees(_T tValue)
	{
		return tValue * static_cast<_T>(IOE_RAD_TO_DEG);
	}

} // namespace Maths
} // namespace IOE