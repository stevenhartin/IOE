#pragma once

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	struct KeyState
	{
		KeyState()
			: OldState(0)
			, NewState(0)
		{
		}

		void Update(std::uint8_t uNewState)
		{
			OldState = NewState;
			NewState = uNewState;
		}

		bool JustPressed() const
		{
			return NewState && (!OldState);
		}

		bool JustReleased() const
		{
			return (!NewState) && OldState;
		}

		bool IsDown() const
		{
			return NewState == 0x1;
		}

		std::uint8_t OldState : 4;
		std::uint8_t NewState : 4;
	};

	//////////////////////////////////////////////////////////////////////////

	// Assuming a British keyboard layout
	enum class EInputKey
	{
		B,
		A,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		Space,
		LCtrl,
		RCtrl,
		Return,
		Enter,
		Backspace,
		LShift,
		RShift,
		Tilde,
		Escape,
		Tab,
		CapsLock,
		LSuper,
		RSuper,
		LAlt,
		RAlt,

		Up,
		Left,
		Down,
		Right,

		Insert,
		Del,
		Home,
		End,
		PgUp,
		PgDown,

		ScrollLock,
		PrintScreen,
		PauseBreak,

		Digit1,
		Digit2,
		Digit3,
		Digit4,
		Digit5,
		Digit6,
		Digit7,
		Digit8,
		Digit9,
		Digit0,

		NumPad0,
		NumPad1,
		NumPad2,
		NumPad3,
		NumPad4,
		NumPad5,
		NumPad6,
		NumPad7,
		NumPad8,
		NumPad9,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,

		NumLock,
		NumPadDivide,
		NumPadMultiply,
		NumPadSubtract,
		NumPadAdd,
		NumPadPeriod,

		LBracket,
		RBracket,
		SemiColon,
		Apostrophe,
		Hash,
		Comma,
		Period,
		ForwardSlash,
		BackwardsSlash,

		KeyCount
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EInputMouseButton
	{
		MouseButton1,
		MouseButton2,
		MouseButton3,
		MouseButton4,
		MouseButton5,
		MouseButton6,
		MouseButton7,
		MouseButton8,

		NumButtons
	};

	//////////////////////////////////////////////////////////////////////////

	enum class EInputMouseAxis
	{
		AxisX,
		AxisY,
		Scroll,

		NumAxes
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE