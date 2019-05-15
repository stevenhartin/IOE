#pragma once

//////////////////////////////////////////////////////////////////////////

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOECore/IOEHighResolutionTime.h>

#include PLATFORM_INCLUDE(IOEHighResolutionTime_Platform.h)

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	// Struct for passing around time deltas. This caches off the inverse
	// delta as it's needed for quite a lot of calculations and thus more
	// optimised to only calculate once instead of constant divides everywhere.
	struct IOETimeDelta
	{
	public:
		IOETimeDelta(float fDelta_)
		{
			SetDelta(fDelta_);
		}

		IOETimeDelta(const IOETimeDelta &) = default;
		IOETimeDelta(IOETimeDelta &&) = default;
		IOETimeDelta &operator=(const IOETimeDelta &) = default;
		IOETimeDelta &operator=(IOETimeDelta &&) = default;

	public:
		// Implicit cast to float
		FORCEINLINE operator float() const
		{
			return fDelta;
		}

		FORCEINLINE IOETimeDelta operator*(float fOther) const
		{
			return IOETimeDelta(fDelta * fOther);
		}

		FORCEINLINE IOETimeDelta &operator*=(float fOther)
		{
			SetDelta(fDelta * fOther);
			return *this;
		}

		FORCEINLINE IOETimeDelta operator+(float fOther) const
		{
			return IOETimeDelta(fDelta + fOther);
		}

		FORCEINLINE IOETimeDelta &operator+=(float fOther)
		{
			SetDelta(fDelta + fOther);
			return *this;
		}

		FORCEINLINE IOETimeDelta operator-(float fOther) const
		{
			return IOETimeDelta(fDelta - fOther);
		}

		FORCEINLINE IOETimeDelta &operator-=(float fOther)
		{
			SetDelta(fDelta - fOther);
			return *this;
		}

		FORCEINLINE IOETimeDelta operator/(float fOther) const
		{
			return IOETimeDelta(fDelta / fOther);
		}

		FORCEINLINE IOETimeDelta &operator/=(float fOther)
		{
			SetDelta(fDelta / fOther);
			return *this;
		}

		FORCEINLINE IOETimeDelta operator/(IOETimeDelta tOther) const
		{
			// More optimised version of divide for TimeDeltas as we already
			// cached
			// off the inverse delta.
			return IOETimeDelta(fDelta * tOther.fInvDelta);
		}

		FORCEINLINE IOETimeDelta &operator/=(IOETimeDelta tOther)
		{
			fDelta *= tOther.fInvDelta;
			fInvDelta *= tOther.fDelta;
			return *this;
		}

	public:
		FORCEINLINE bool IsPaused()
		{
			return fDelta > 0.0f;
		}

		FORCEINLINE void SetDelta(float fDelta_)
		{
			fDelta = fDelta_;
			if (fDelta > 0.0f)
			{
				fInvDelta = 1.0f / fDelta;
			}
			else
			{
				fInvDelta = std::numeric_limits<float>::infinity();
			}
		}

		float fDelta;
		float fInvDelta;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEHighResolutionTime
	{
	public:
		// Default copy / move constructor and assignment / move operators
		IOEHighResolutionTime() = default;

		IOEHighResolutionTime(const IOEHighResolutionTime &) = default;
		IOEHighResolutionTime(IOEHighResolutionTime &&) = default;

		IOEHighResolutionTime &
		operator=(const IOEHighResolutionTime &) = default;
		IOEHighResolutionTime &operator=(IOEHighResolutionTime &&) = default;

	public:
		FORCEINLINE double GetRawTime() const
		{
			return m_tPlatform.GetRawTime();
		}
		FORCEINLINE double operator-(const IOEHighResolutionTime &rhs) const
		{
			return m_tPlatform.GetTimeDiff(rhs.GetRawTime());
		}

	private:
		IOEHighResolutionTime_Platform m_tPlatform;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE