#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>
#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>

#include "IOEInputDevice_PlatformBase.h"

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

	enum class EInputMouseButton;
	enum class EInputMouseAxis;

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEInputMouse_PlatformBase
		: public IOEInputDevice_PlatformBase
	{
	public:
		IOEInputMouse_PlatformBase(IOEInputDevice & rDevice)
			: IOEInputDevice_PlatformBase(rDevice)
		{
		}

		virtual bool IsKeyDown(EInputMouseButton eKey) const = 0;
		virtual bool JustPressed(EInputMouseButton eKey) const = 0;
		virtual bool JustReleased(EInputMouseButton eKey) const = 0;
		virtual float GetFrameDelta(EInputMouseAxis eAxis) const = 0;

		virtual void SetCursorPos(std::int32_t nX, std::int32_t nY) const = 0;
		virtual void GetCursorPos(std::int32_t & nX, std::int32_t & nY)
			const = 0;
		virtual void SetCursorVisibility(bool bVisible) const = 0;
		virtual void SnapToCentre() const = 0;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE