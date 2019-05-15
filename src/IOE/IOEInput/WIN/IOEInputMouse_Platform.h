#pragma once

//////////////////////////////////////////////////////////////////////////

#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <IOE/IOECore/IOEDefines.h>

#include "../IOEInputMouse_PlatformBase.h"
#include "../IOEInputKeys.h"

//////////////////////////////////////////////////////////////////////////

using std::vector;
using std::string;
using std::map;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	class IOEInputMouse_Platform : public IOEInputMouse_PlatformBase
	{
	public:
		IOEInputMouse_Platform(IOEInputDevice &rDevice)
			: IOEInputMouse_PlatformBase(rDevice)
		{
		}

		virtual void SetUp() override;
		virtual void Process() override;

		FORCEINLINE virtual bool
		IsKeyDown(EInputMouseButton eKey) const override final
		{
			return m_arrButtons[(uint32_t)eKey].IsDown();
		}

		FORCEINLINE virtual bool
		JustPressed(EInputMouseButton eKey) const override final
		{
			return m_arrButtons[(uint32_t)eKey].JustPressed();
		}

		FORCEINLINE virtual bool
		JustReleased(EInputMouseButton eKey) const override final
		{
			return m_arrButtons[(uint32_t)eKey].JustReleased();
		}

		FORCEINLINE virtual float
		GetFrameDelta(EInputMouseAxis eAxis) const override final
		{
			return m_arrAxes[(uint32_t)eAxis];
		}

		virtual void SetCursorPos(std::int32_t uX,
								  std::int32_t uY) const override final;
		virtual void GetCursorPos(std::int32_t &nX,
								  std::int32_t &nY) const override final;
		virtual void SetCursorVisibility(bool bVisible) const override final;
		virtual void SnapToCentre() const override final;

	private:
		LPDIRECTINPUTDEVICE8 m_pDevice;

		KeyState m_arrButtons[EInputMouseButton::NumButtons];
		float m_arrAxes[EInputMouseAxis::NumAxes];
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE