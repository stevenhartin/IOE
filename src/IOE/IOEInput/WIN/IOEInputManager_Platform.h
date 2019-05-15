#if !defined(DIRECTINPUT_VERSION)
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include "../IOEInputManager_PlatformBase.h"

namespace IOE
{
namespace Input
{

	//////////////////////////////////////////////////////////////////////////

	struct IDirectInput;

	//////////////////////////////////////////////////////////////////////////

	class IOEInputManager_Platform : public IOEInputManager_PlatformBase
	{
	public:
		virtual void Setup() override;

		LPDIRECTINPUT8 GetDirectInput()
		{
			return m_pDirectInput;
		}
		const LPDIRECTINPUT8 GetDirectInput() const
		{
			return m_pDirectInput;
		}

	private:
		LPDIRECTINPUT8 m_pDirectInput;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE