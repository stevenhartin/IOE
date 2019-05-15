#include "IOEVertexFormat_Platform.h"
#include "../IOEVertexFormat.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	std::vector<D3D11_INPUT_ELEMENT_DESC>
	IOEVertexFormat_Platform::GetInputDescription()
	{
		std::vector<D3D11_INPUT_ELEMENT_DESC> arrInputs;
		for (IOESemanticFormat &tInput : m_rFormat.m_arrSemantics)
		{
			D3D11_INPUT_ELEMENT_DESC tDesc{
				tInput.m_szSemanticName.c_str(), tInput.m_uSemanticIndex,
				GetFormat(tInput.m_eTextureFormat), tInput.m_uSlotIndex,
				static_cast<std::uint32_t>(tInput.m_uOffset),
				tInput.m_eClassification == EVertexClassification::PerVertex
					? D3D11_INPUT_PER_VERTEX_DATA
					: D3D11_INPUT_PER_INSTANCE_DATA,
				tInput.m_uInstanceDataStepRate
			};
			arrInputs.push_back(tDesc);
		}
		return arrInputs;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE