#pragma once

//////////////////////////////////////////////////////////////////////////

#include <IOE/IOERenderer/IOEViewport_PlatformBase.h>

//////////////////////////////////////////////////////////////////////////

struct ID3DViewport;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEViewport_Platform : public IOEViewport_PlatformBase
	{
	public:
		virtual void Init(float fWidth, float fHeight) override final
		{
		}

	private:
	};

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE