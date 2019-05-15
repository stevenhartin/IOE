#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>

#include <d3d11.h>

#include "../IOEVertexFormat_PlatformBase.h"

//////////////////////////////////////////////////////////////////////////

using std::vector;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEVertexFormat;

	//////////////////////////////////////////////////////////////////////////

	class IOEVertexFormat_Platform : public IOEVertexFormat_PlatformBase
	{
	public:
		IOEVertexFormat_Platform(IOEVertexFormat &rFormat)
			: IOEVertexFormat_PlatformBase(rFormat)
		{
		}

		std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputDescription();

	private:
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE