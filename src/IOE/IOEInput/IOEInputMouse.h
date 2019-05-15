#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>
#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>

#include "IOEInputDevice.h"

#include PLATFORM_INCLUDE(IOEInputMouse_Platform.h)

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::string;
using std::map;
using std::int32_t;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	class IOEInputMouse : public IOEInputDevice
	{
	public:
		static const EInputDeviceType ms_DeviceType = EInputDeviceType::Mouse;

	public:
		IOEInputMouse(EInputDeviceType eDeviceType = ms_DeviceType)
			: IOEInputDevice(eDeviceType)
			, m_tPlatform(*this)
			, m_bLocked(false)
			, m_nLastX(0)
			, m_nLastY(0)
		{
		}

		FORCEINLINE virtual void SetUp() override final
		{
			m_tPlatform.SetUp();
			LockCursor();
		}

		virtual void Process() override final;

		FORCEINLINE bool IsKeyDown(EInputMouseButton eKey) const
		{
			return m_tPlatform.IsKeyDown(eKey);
		}
		FORCEINLINE bool JustReleased(EInputMouseButton eKey) const
		{
			return m_tPlatform.JustReleased(eKey);
		}
		FORCEINLINE bool JustPressed(EInputMouseButton eKey) const
		{
			return m_tPlatform.JustPressed(eKey);
		}

		FORCEINLINE float GetFrameDelta(EInputMouseAxis eAxis) const
		{
			return m_tPlatform.GetFrameDelta(eAxis);
		}

	public:
		FORCEINLINE void LockCursor()
		{
			if (!m_bLocked)
			{
				m_tPlatform.GetCursorPos(m_nLastX, m_nLastY);
				m_tPlatform.SetCursorVisibility(false);
				m_bLocked = true;
			}
		}

		FORCEINLINE void UnlockCursor()
		{
			if (m_bLocked)
			{
				m_tPlatform.SetCursorPos(m_nLastX, m_nLastY);
				m_tPlatform.SetCursorVisibility(true);
				m_bLocked = false;
			}
		}

		FORCEINLINE bool IsLocked() const
		{
			return m_bLocked;
		}

		// If window space is true 0,0 is top-left of window, otherwise 0,0 is
		// top-left of screen
		void GetAbsolutePosition(int32_t &nX, int32_t &nY,
								 bool bWindowSpace = true) const;

	private:
		IOEInputMouse_Platform m_tPlatform;

		bool m_bLocked;

		// Remembered cursor position between locking and unlocking
		int32_t m_nLastX, m_nLastY;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE