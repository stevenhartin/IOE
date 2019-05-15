#pragma once

#include <IOE/IOECore/IOEHighResolutionTime_PlatformBase.h>
#include <IOE/IOEExceptions/IOEExceptionList.h>

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	class IOEHighResolutionTime_Platform
		: public IOEHighResolutionTime_PlatformBase
	{
	public:
		// Default copy / move constructor and assignment / move operators
		IOEHighResolutionTime_Platform(
			const IOEHighResolutionTime_Platform &) = default;
		IOEHighResolutionTime_Platform(IOEHighResolutionTime_Platform &&) =
			default;

		IOEHighResolutionTime_Platform &
		operator=(const IOEHighResolutionTime_Platform &) = default;
		IOEHighResolutionTime_Platform &
		operator=(IOEHighResolutionTime_Platform &&) = default;

	public:
		IOEHighResolutionTime_Platform();

		/************************************************************************/
		/* Obtains  the raw time (in ms) since this application was booted
		/* up. This does not take into account any paused time or time
		dilation.
		/*
		/* @return double The amount of time (in ms) since the application
		/* started.
		/************************************************************************/
		FORCEINLINE virtual double GetRawTime() const final override
		{
			return m_dRawTime;
		}

		/************************************************************************/
		/* Obtains the difference in time between two raw time snapshots.
		/*
		/* @return double The amount of difference in time (in ms)
		/************************************************************************/
		FORCEINLINE virtual double
		GetTimeDiff(double dTime) const final override
		{
			return GetRawTime() - dTime;
		}
		FORCEINLINE double
		operator-(const IOEHighResolutionTime_Platform &rhs) const
		{
			return GetRawTime() - rhs.GetRawTime();
		}

	private:
	private:
		static double ms_dInvFrequency;
		double m_dRawTime;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE