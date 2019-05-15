#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOECore/IOEManagerContainer.h>
#include <IOE/IOEApplication/IOEApplication.h>

#include "../IOEInputManager.h"
#include "IOEInputKeyboard_Platform.h"
#include "../IOEInputKeyboard.h"

namespace IOE
{
namespace Input
{

	std::int32_t IOEInputKeyboard_Platform::GetPlatformKeyCode(EInputKey eKey)
	{
		switch (eKey)
		{
		case EInputKey::A:
			return DIK_A;
		case EInputKey::B:
			return DIK_B;
		case EInputKey::C:
			return DIK_C;
		case EInputKey::D:
			return DIK_D;
		case EInputKey::E:
			return DIK_D;
		case EInputKey::F:
			return DIK_F;
		case EInputKey::G:
			return DIK_G;
		case EInputKey::H:
			return DIK_H;
		case EInputKey::I:
			return DIK_I;
		case EInputKey::J:
			return DIK_J;
		case EInputKey::K:
			return DIK_K;
		case EInputKey::L:
			return DIK_L;
		case EInputKey::M:
			return DIK_M;
		case EInputKey::N:
			return DIK_N;
		case EInputKey::O:
			return DIK_O;
		case EInputKey::P:
			return DIK_P;
		case EInputKey::Q:
			return DIK_Q;
		case EInputKey::R:
			return DIK_R;
		case EInputKey::S:
			return DIK_S;
		case EInputKey::T:
			return DIK_T;
		case EInputKey::U:
			return DIK_U;
		case EInputKey::V:
			return DIK_V;
		case EInputKey::W:
			return DIK_W;
		case EInputKey::X:
			return DIK_X;
		case EInputKey::Y:
			return DIK_Y;
		case EInputKey::Z:
			return DIK_Z;

		case EInputKey::Space:
			return DIK_SPACE;
		case EInputKey::LCtrl:
			return DIK_LCONTROL;
		case EInputKey::RCtrl:
			return DIK_RCONTROL;
		case EInputKey::Return:
			return DIK_RETURN;
		case EInputKey::Enter:
			return DIK_NUMPADENTER;
		case EInputKey::Backspace:
			return DIK_BACKSPACE;
		case EInputKey::LShift:
			return DIK_LSHIFT;
		case EInputKey::RShift:
			return DIK_RSHIFT;
		case EInputKey::Tilde:
			return DIK_GRAVE;
		case EInputKey::Escape:
			return DIK_ESCAPE;
		case EInputKey::Tab:
			return DIK_TAB;
		case EInputKey::CapsLock:
			return DIK_CAPSLOCK;
		case EInputKey::LSuper:
			return DIK_LWIN;
		case EInputKey::RSuper:
			return DIK_RWIN;
		case EInputKey::LAlt:
			return DIK_LALT;
		case EInputKey::RAlt:
			return DIK_RALT;

		case EInputKey::Up:
			return DIK_UPARROW;
		case EInputKey::Left:
			return DIK_LEFTARROW;
		case EInputKey::Down:
			return DIK_DOWNARROW;
		case EInputKey::Right:
			return DIK_RIGHTARROW;

		case EInputKey::Insert:
			return DIK_INSERT;
		case EInputKey::Del:
			return DIK_DELETE;
		case EInputKey::Home:
			return DIK_HOME;
		case EInputKey::End:
			return DIK_END;
		case EInputKey::PgUp:
			return DIK_PGUP;
		case EInputKey::PgDown:
			return DIK_PGDN;

		case EInputKey::ScrollLock:
			return DIK_SCROLL;
		case EInputKey::PrintScreen:
			return DIK_SYSRQ; // TODO: Check this DIK is correct
		case EInputKey::PauseBreak:
			return DIK_PAUSE;

		case EInputKey::Digit1:
			return DIK_1;
		case EInputKey::Digit2:
			return DIK_2;
		case EInputKey::Digit3:
			return DIK_3;
		case EInputKey::Digit4:
			return DIK_4;
		case EInputKey::Digit5:
			return DIK_5;
		case EInputKey::Digit6:
			return DIK_6;
		case EInputKey::Digit7:
			return DIK_7;
		case EInputKey::Digit8:
			return DIK_8;
		case EInputKey::Digit9:
			return DIK_9;
		case EInputKey::Digit0:
			return DIK_0;

		case EInputKey::NumPad0:
			return DIK_NUMPAD0;
		case EInputKey::NumPad1:
			return DIK_NUMPAD1;
		case EInputKey::NumPad2:
			return DIK_NUMPAD2;
		case EInputKey::NumPad3:
			return DIK_NUMPAD3;
		case EInputKey::NumPad4:
			return DIK_NUMPAD4;
		case EInputKey::NumPad5:
			return DIK_NUMPAD5;
		case EInputKey::NumPad6:
			return DIK_NUMPAD6;
		case EInputKey::NumPad7:
			return DIK_NUMPAD7;
		case EInputKey::NumPad8:
			return DIK_NUMPAD8;
		case EInputKey::NumPad9:
			return DIK_NUMPAD9;

		case EInputKey::F1:
			return DIK_F1;
		case EInputKey::F2:
			return DIK_F2;
		case EInputKey::F3:
			return DIK_F3;
		case EInputKey::F4:
			return DIK_F4;
		case EInputKey::F5:
			return DIK_F5;
		case EInputKey::F6:
			return DIK_F6;
		case EInputKey::F7:
			return DIK_F7;
		case EInputKey::F8:
			return DIK_F8;
		case EInputKey::F9:
			return DIK_F9;
		case EInputKey::F10:
			return DIK_F10;
		case EInputKey::F11:
			return DIK_F11;
		case EInputKey::F12:
			return DIK_F12;

		case EInputKey::NumLock:
			return DIK_NUMLOCK;
		case EInputKey::NumPadDivide:
			return DIK_NUMPADSLASH;
		case EInputKey::NumPadMultiply:
			return DIK_NUMPADSTAR;
		case EInputKey::NumPadSubtract:
			return DIK_NUMPADMINUS;
		case EInputKey::NumPadAdd:
			return DIK_NUMPADPLUS;
		case EInputKey::NumPadPeriod:
			return DIK_NUMPADPERIOD;

		case EInputKey::LBracket:
			return DIK_LBRACKET;
		case EInputKey::RBracket:
			return DIK_RBRACKET;
		case EInputKey::SemiColon:
			return DIK_SEMICOLON;
		case EInputKey::Apostrophe:
			return DIK_APOSTROPHE;
		case EInputKey::Comma:
			return DIK_COMMA;
		case EInputKey::Period:
			return DIK_PERIOD;
		case EInputKey::ForwardSlash:
			return DIK_SLASH;
		case EInputKey::BackwardsSlash:
			return DIK_BACKSLASH;

		case EInputKey::Hash: /*return DIK_;*/
		default:
			return -1;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputKeyboard_Platform::SetUp()
	{
		auto pDirectInput(IOEInputManager::GetSingletonPtr()
							  ->GetPlatform()
							  .GetDirectInput());
		pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
		m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);

		m_pKeyboard->SetCooperativeLevel(IOE::Core::g_pEngine->GetApplication()
											 ->GetPlatform()
											 .GetApplicationWindow(),
										 DISCL_FOREGROUND |
											 DISCL_NONEXCLUSIVE);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEInputKeyboard_Platform::Process()
	{
		char arrKeyboardState[256];
		HRESULT hr;

		// Clear the keyboard data buffer - just in case.
		ZeroMemory(arrKeyboardState, 256);

		hr = m_pKeyboard->GetDeviceState(sizeof(arrKeyboardState),
										 (LPVOID)&arrKeyboardState);
		if (FAILED(hr))
		{
			// If this failed, the device has probably been lost.
			// Check for (hr == DIERR_INPUTLOST) and attempt to reacquire it
			// here.

			hr = m_pKeyboard->Acquire();
			while (hr == DIERR_INPUTLOST)
			{
				hr = m_pKeyboard->Acquire();
			}

			hr = m_pKeyboard->GetDeviceState(sizeof(arrKeyboardState),
											 (LPVOID)&arrKeyboardState);
		}

		if (!FAILED(hr))
		{
			// If we can find the keyboard device

			for (uint32_t eKey = 0; eKey < (uint32_t)EInputKey::KeyCount;
				 ++eKey)
			{
				int32_t nCode(GetPlatformKeyCode((EInputKey)eKey));
				if (nCode >= 0)
				{
					m_arrStates[eKey].Update(
						(arrKeyboardState[nCode] & 0x80) != 0);
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

} // Input
} // IOE