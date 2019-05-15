#include "IOEHighResolutionTime_Platform.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <limits>

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	double IOEHighResolutionTime_Platform::ms_dInvFrequency = -1.0;

	//////////////////////////////////////////////////////////////////////////

	IOEHighResolutionTime_Platform::IOEHighResolutionTime_Platform()
		: m_dRawTime(0.0)
	{
		LARGE_INTEGER lnFrequency;
		if (ms_dInvFrequency < 0.0)
		{
			if (QueryPerformanceFrequency(&lnFrequency))
			{
				// Obtain amount cycles per 1000 units (ms).
				ms_dInvFrequency =
					1000.0 / static_cast<double>(lnFrequency.QuadPart);
			}
			else
			{
				ms_dInvFrequency = std::numeric_limits<double>::infinity();
				THROW_IOE_BASE_EXCEPTION(
					"Could not obtain the performance frequency",
					E_ERROR_EXTERNAL_ERROR);
			}
		}

		LARGE_INTEGER lnCounter;
		if (QueryPerformanceCounter(&lnCounter))
		{
			m_dRawTime =
				static_cast<double>(lnCounter.QuadPart) * ms_dInvFrequency;
		}
		else
		{
			THROW_IOE_BASE_EXCEPTION(
				"Could not obtain the performance counter",
				E_ERROR_EXTERNAL_ERROR);
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE