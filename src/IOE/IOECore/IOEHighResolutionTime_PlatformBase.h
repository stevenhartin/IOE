#pragma once

namespace IOE
{
namespace Core
{

	//////////////////////////////////////////////////////////////////////////

	class IOEHighResolutionTime_PlatformBase
	{
	public:
		virtual double GetTimeDiff(double dTime) const = 0;
		virtual double GetRawTime() const = 0;

	private:
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Core
} // namespace IOE