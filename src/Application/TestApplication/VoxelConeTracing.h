#pragma once

#include <memory>

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOERenderer/IOEBuffer.h>
#include <IOE/IOERenderer/IOEVisibilityBounds.h>

#include "Axis.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{
	class IOEMaterial;
	class IOEFont;
	class IOEText;
}
namespace Application
{
	struct C_CommandLine;
} // namespace Application
} // namespace IOE

//////////////////////////////////////////////////////////////////////////

struct DebugOctreeVertex
{
	DebugOctreeVertex();

	DebugOctreeVertex(const IOE::Maths::IOEVector &vPos_, float fX, float fY,
					  float fZ);

	void Set(const IOE::Maths::IOEVector &vPos_, float fX, float fY, float fZ);

	IOE::Maths::IOEVector vPos;
	float arrOffset[3];
};

//////////////////////////////////////////////////////////////////////////

class VoxelConeTracer
{
	static const int32_t ms_nVoxelTreeSize = 128;
	static const float ms_fVoxelTreeSize;

public:
	VoxelConeTracer();

	void Initialise();

	void PreRender(IOE::Renderer::IOERPI *RPI);
	void Render(IOE::Renderer::IOERPI *RPI);
	void PostRender(IOE::Renderer::IOERPI *RPI);
	void FinishRender(IOE::Renderer::IOERPI *RPI);

	void ModifyLOD(float fDifference);
	void ModifyAxis(int32_t nDiff);

public:
	void SetShadowMap(IOE::Renderer::IOETexture *pShadowMap);

	void ToggleDebugDraw()
	{
		m_bDrawDebug = !m_bDrawDebug;
	}
	bool IsDebugDrawEnabled() const
	{
		return m_bDrawDebug;
	}

	void UpdateLightSettings();

private:
	void RenderStaticLighting(IOE::Renderer::IOERPI *RPI);
	void CreateMips(IOE::Renderer::IOERPI *RPI);
	void DebugDrawScene(IOE::Renderer::IOERPI *RPI);

private:
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pBuildOctreeMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pDebugOctreeMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pDebugOctreePointsMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pGenerateMipsMaterial[EAxisDirection::NumDirections];

	IOE::Maths::IOEVector m_vOctreeOffset;
	IOE::Maths::IOEVector m_vOctreeScale;
	IOE::Maths::IOEVector m_vInvOctreeScale;

	IOE::Renderer::IOETexture m_tVoxelDepthTarget;
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_tVoxelRenderTarget[EAxisDirection::NumDirections];
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_pVoxelRenderTargetTextureBind[EAxisDirection::NumDirections];

#if (IOE_SHADER_DEBUG == IOE_ON)
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_arrUAVs[EAxisDirection::NumDirections]
				 [MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize) + 1][5];
#else
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_arrUAVs[EAxisDirection::NumDirections]
				 [MOST_SIGNIFICANT_BIT(ms_nVoxelTreeSize) + 1][2];
#endif

	std::shared_ptr<IOE::Renderer::IOETexture>
		m_arrTextures[EAxisDirection::NumDirections];

	IOE::Renderer::IOEBuffer<DebugOctreeVertex> m_tVertexBuffer;

	EAxisDirection::Enum m_eDirection;

	bool m_bDrawDebug;

	float m_fLod;

	std::shared_ptr<IOE::Renderer::IOESamplerState> m_pClampSampler;
};

//////////////////////////////////////////////////////////////////////////
