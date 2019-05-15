#pragma once

//////////////////////////////////////////////////////////////////////////

#include <IOE/IOECore/IOEDefines.h>

#include "IOEInputDevice.h"
#include "IOEInputKeys.h"

#include PLATFORM_INCLUDE(IOEInputKeyboard_Platform.h)

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	class IOEInputKeyboard : public IOEInputDevice
	{
	public:
		static const EInputDeviceType ms_DeviceType =
			EInputDeviceType::Keyboard;

	public:
		IOEInputKeyboard(EInputDeviceType eDeviceType = ms_DeviceType)
			: IOEInputDevice(eDeviceType)
			, m_tPlatform(*this)
		{
		}

		FORCEINLINE virtual void SetUp() override final
		{
			m_tPlatform.SetUp();
		}

		FORCEINLINE virtual void Process() override final
		{
			m_tPlatform.Process();
		}

		FORCEINLINE bool IsKeyDown(EInputKey eKey) const
		{
			return m_tPlatform.IsKeyDown(eKey);
		}
		FORCEINLINE bool JustReleased(EInputKey eKey) const
		{
			return m_tPlatform.JustReleased(eKey);
		}
		FORCEINLINE bool JustPressed(EInputKey eKey) const
		{
			return m_tPlatform.JustPressed(eKey);
		}

	private:
		IOEInputKeyboard_Platform m_tPlatform;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE