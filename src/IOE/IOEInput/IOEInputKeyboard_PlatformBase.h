#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>

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

	enum class EInputKey;

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEInputKeyboard_PlatformBase
		: public IOEInputDevice_PlatformBase
	{
	public:
		IOEInputKeyboard_PlatformBase(class IOEInputDevice & rDevice)
			: IOEInputDevice_PlatformBase(rDevice)
		{
		}

		virtual bool IsKeyDown(EInputKey eKey) const = 0;
		virtual bool JustPressed(EInputKey eKey) const = 0;
		virtual bool JustReleased(EInputKey eKey) const = 0;
	};

	/////////////////////////////////////////////////////////////////////////

} // namespace Input
} // namespace IOE