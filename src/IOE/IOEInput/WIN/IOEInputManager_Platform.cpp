#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOEApplication/IOEApplication.h>

#include "IOEInputManager_Platform.h"

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	void IOEInputManager_Platform::Setup()
	{
		IOE::Application::IOEApplication_Platform &rAppPlatform(
			IOE::Core::g_pEngine->GetApplication()->GetPlatform());

		DirectInput8Create(rAppPlatform.GetApplicationHandle(),
						   DIRECTINPUT_VERSION, IID_IDirectInput8,
						   reinterpret_cast<void **>(&m_pDirectInput), NULL);
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE