#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOECore/IOEManagerContainer.h>

#include <IOE/IOEApplication/IOEApplication.h>

#include "../IOEInputManager.h"
#include "../IOEInputDevice.h"
#include "../IOEInputMouse.h"
#include "IOEInputMouse_Platform.h"

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::SetUp()
	{
		auto pDirectInput(IOEInputManager::GetSingletonPtr()
							  ->GetPlatform()
							  .GetDirectInput());
		if (FAILED(
				pDirectInput->CreateDevice(GUID_SysMouse, &m_pDevice, NULL)))
		{
			// The device cannot be created
			m_rDevice.Disable();
			m_rDevice.CleanUp();
		}
		else
		{
			if (SUCCEEDED(m_pDevice->SetDataFormat(&c_dfDIMouse2)))
			{
				// The format of the device cannot be assigned
				m_rDevice.Disable();
				m_rDevice.CleanUp();
			}
			else
			{
				auto pHandle(IOE::Core::g_pEngine->GetApplication()
								 ->GetPlatform()
								 .GetApplicationWindow());
				// NOTE: To hide cursor, change NONEXCLUSIVE to EXCLUSIVE
				if (FAILED(m_pDevice->SetCooperativeLevel(
						pHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
				{
					// We cannot obtain a lock on the mouse.
					m_rDevice.Disable();
					m_rDevice.CleanUp();
				}
				else
				{
					// Device successfully obtained and locked.
					m_rDevice.Enable();
					m_pDevice->Acquire();
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::Process()
	{
		DIMOUSESTATE2 tMouseState;
		HRESULT tResult;

		// Clear the keyboard data buffer - just in case.
		ZeroMemory(&tMouseState, sizeof(DIMOUSESTATE2));

		tResult =
			m_pDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &tMouseState);
		if (FAILED(tResult))
		{
			// If this failed, the device has probably been lost.
			// Check for (hr == DIERR_INPUTLOST) and attempt to reacquire it
			// here.

			tResult = m_pDevice->Acquire();
			while (tResult == DIERR_INPUTLOST)
			{
				tResult = m_pDevice->Acquire();
			}

			tResult =
				m_pDevice->GetDeviceState(sizeof(DIMOUSESTATE2), &tMouseState);
		}

		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton1].Update(
			(tMouseState.rgbButtons[0] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton2].Update(
			(tMouseState.rgbButtons[1] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton3].Update(
			(tMouseState.rgbButtons[2] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton4].Update(
			(tMouseState.rgbButtons[3] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton5].Update(
			(tMouseState.rgbButtons[4] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton6].Update(
			(tMouseState.rgbButtons[5] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton7].Update(
			(tMouseState.rgbButtons[6] & 0x80) != 0);
		m_arrButtons[(std::uint32_t)EInputMouseButton::MouseButton8].Update(
			(tMouseState.rgbButtons[7] & 0x80) != 0);

		m_arrAxes[(std::uint32_t)EInputMouseAxis::AxisX] =
			static_cast<float>(tMouseState.lX);
		m_arrAxes[(std::uint32_t)EInputMouseAxis::AxisY] =
			static_cast<float>(tMouseState.lX);
		m_arrAxes[(std::uint32_t)EInputMouseAxis::Scroll] =
			static_cast<float>(tMouseState.lX);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::SetCursorPos(std::int32_t nX,
											  std::int32_t nY) const
	{
		::SetCursorPos(nX, nY);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::GetCursorPos(std::int32_t &nX,
											  std::int32_t &nY) const
	{
		POINT tPoint;
		::GetCursorPos(&tPoint);
		::ScreenToClient(IOE::Core::g_pEngine->GetApplication()
							 ->GetPlatform()
							 .GetApplicationWindow(),
						 &tPoint);
		nX = tPoint.x;
		nY = tPoint.y;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::SetCursorVisibility(bool bVisible) const
	{
		::ShowCursor(bVisible ? TRUE : FALSE);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputMouse_Platform::SnapToCentre() const
	{
		std::uint32_t uWidth(0), uHeight(0);
		std::uint32_t uX(0), uY(0);
		IOE::Core::g_pEngine->GetApplication()->GetWindowPos(uX, uY);
		IOE::Core::g_pEngine->GetApplication()->GetWindowSize(uWidth, uHeight);
		uWidth >>= 1;
		uHeight >>= 1;

		SetCursorPos((int32_t)(uX + uWidth), (int32_t)(uY + uHeight));
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE