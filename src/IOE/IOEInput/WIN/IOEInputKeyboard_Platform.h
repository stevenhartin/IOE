#pragma once

//////////////////////////////////////////////////////////////////////////

#include <cstdint>

#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <IOE/IOECore/IOEDefines.h>

#include "../IOEInputKeyboard_PlatformBase.h"
#include "../IOEInputKeys.h"

//////////////////////////////////////////////////////////////////////////

struct IDirectInputDevice;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Input
{

	static_assert(sizeof(KeyState) == 1, "Bit fields not supported");

	class IOEInputKeyboard_Platform : public IOEInputKeyboard_PlatformBase
	{
	public:
		IOEInputKeyboard_Platform(class IOEInputDevice &rDevice)
			: IOEInputKeyboard_PlatformBase(rDevice)
		{
		}

		virtual void SetUp() override final;
		virtual void Process() override final;

		FORCEINLINE virtual bool IsKeyDown(EInputKey eKey) const override final
		{
			return m_arrStates[(uint32_t)eKey].IsDown();
		}

		FORCEINLINE virtual bool
		JustPressed(EInputKey eKey) const override final
		{
			return m_arrStates[(uint32_t)eKey].JustPressed();
		}

		FORCEINLINE virtual bool
		JustReleased(EInputKey eKey) const override final
		{
			return m_arrStates[(uint32_t)eKey].JustReleased();
		}

	private:
		KeyState m_arrStates[EInputKey::KeyCount];

		LPDIRECTINPUTDEVICE8 m_pKeyboard;

		static std::int32_t GetPlatformKeyCode(EInputKey eKey);
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE