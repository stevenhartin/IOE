#include "IOEVertexFormat.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	IOESemanticFormat::IOESemanticFormat(std::string szSemanticName,
										 std::uint32_t uSemanticIndex,
										 ETextureFormat eTextureFormat,
										 std::uint32_t uSlotIndex,
										 std::size_t uOffset,
										 EVertexClassification eClassification,
										 std::uint32_t uInstanceDataStepRate)
		: m_szSemanticName(szSemanticName)
		, m_uSemanticIndex(uSemanticIndex)
		, m_eTextureFormat(eTextureFormat)
		, m_uSlotIndex(uSlotIndex)
		, m_uOffset(uOffset)
		, m_eClassification(eClassification)
		, m_uInstanceDataStepRate(uInstanceDataStepRate)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEVertexFormatContainer::OnManagerPreInit()
	{
		auto pVertexFormat(CreateVertexFormat("Default"));
		pVertexFormat->AddSemanticFormat("POSITION", 0,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(ModelVertex, m_vPosition),
										 EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("NORMAL", 0,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(ModelVertex, m_vNormal),
										 EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("COLOR", 0,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(ModelVertex, m_vColour),
										 EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat(
			"TEXCOORD", 0, ETextureFormat::R32G32_FLOAT, 0,
			offsetof(ModelVertex, m_vUV), EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("TEXCOORD", 1,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(ModelVertex, m_vTangent),
										 EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("TEXCOORD", 2,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(ModelVertex, m_vBinormal),
										 EVertexClassification::PerVertex, 0);

		pVertexFormat = CreateVertexFormat("DebugDraw");
		pVertexFormat->AddSemanticFormat(
			"POSITION", 0, ETextureFormat::R32G32B32A32_FLOAT, 0,
			offsetof(DebugVertex, vPos), EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("COLOR", 0,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(DebugVertex, vColour),
										 EVertexClassification::PerVertex, 0);

		pVertexFormat = CreateVertexFormat("Font");
		pVertexFormat->AddSemanticFormat(
			"POSITION", 0, ETextureFormat::R32G32B32A32_FLOAT, 0,
			offsetof(FontVertex, vPos), EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat("COLOR", 0,
										 ETextureFormat::R32G32B32A32_FLOAT, 0,
										 offsetof(FontVertex, vColour),
										 EVertexClassification::PerVertex, 0);
		pVertexFormat->AddSemanticFormat(
			"TEXCOORD", 0, ETextureFormat::R32G32_FLOAT, 0,
			offsetof(FontVertex, fUV), EVertexClassification::PerVertex, 0);
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE