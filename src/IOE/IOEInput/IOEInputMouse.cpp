#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOECore/Algorithms/IOEStringHelpers.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include "IOEInputMouse.h"

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse::GetAbsolutePosition(int32_t &nX, int32_t &nY,
											bool bWindowSpace /*=true*/) const
	{
		std::uint32_t uX, uY;
		m_tPlatform.GetCursorPos(nX, nY);
		if (bWindowSpace)
		{
			IOE::Core::g_pEngine->GetApplication()->GetWindowPos(uX, uY);
			nX -= static_cast<int32_t>(uX);
			nY -= static_cast<int32_t>(uY);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse::Process()
	{
		m_tPlatform.Process();
		if (m_bLocked)
		{
			if (IOE::Core::g_pEngine->GetApplication()->HasFocus())
			{
				// m_tPlatform.SnapToCentre();
			}
			else
			{
				m_tPlatform.SetCursorVisibility(true);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE