#pragma once

//////////////////////////////////////////////////////////////////////////

#include <IOE/IOECore/IOEDefines.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEVertexFormat;

	//////////////////////////////////////////////////////////////////////////

	interface_class IOEVertexFormat_PlatformBase
	{
	public:
		IOEVertexFormat_PlatformBase(IOEVertexFormat & rFormat)
			: m_rFormat(rFormat)
		{
		}

	protected:
		IOEVertexFormat &m_rFormat;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE