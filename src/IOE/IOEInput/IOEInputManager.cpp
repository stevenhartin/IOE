#include "IOEInputManager.h"
#include "IOEInputDevice.h"
#include "IOEInputKeyboard.h"
#include "IOEInputMouse.h"

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	void IOEInputManager::OnManagerPreInit()
	{
		m_tPlatform.Setup();
		CreateDevice<IOEInputKeyboard>();
		CreateDevice<IOEInputMouse>();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputManager::OnPreUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		for (auto pDevice : m_arrDevices)
			if (pDevice->IsEnabled())
				pDevice->Process();
	}

	//////////////////////////////////////////////////////////////////////////

} // namepsace Input
} // namespace IOE