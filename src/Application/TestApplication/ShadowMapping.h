#pragma once

//////////////////////////////////////////////////////////////////////////

#include <memory>

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOERenderer/IOEBuffer.h>
#include <IOE/IOERenderer/IOEVisibilityBounds.h>

//////////////////////////////////////////////////////////////////////////

#define DUAL_PARABALOID_SHADOW_MAPPING 1

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

class CubeShadowMap
{
	static const int32_t ms_nShadowMapSize = 1024;

	float m_fShadowMapSize;

public:
	CubeShadowMap(float fResolution = (float)ms_nShadowMapSize);
	void SetResolution(float fResolution);

	void Initialise();

	void PreRender(IOE::Renderer::IOERPI *RPI);
	void Render(IOE::Renderer::IOERPI *RPI);
	void PostRender(IOE::Renderer::IOERPI *RPI);

	void SetLightPosition(const IOE::Maths::IOEVector &vLightPos);

	IOE::Renderer::IOETexture *GetShadowMap()
	{
		return m_pShaderResources.get();
	}

private:
	std::shared_ptr<IOE::Renderer::IOETexture> m_pShadowMap;
	std::shared_ptr<IOE::Renderer::IOETexture> m_pShaderResources;

	// Cuboid shadow mapping, render scene 6 times in all axes to a texture
	// cube
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_arrRenderTargets[IOE::Renderer::ECubeFace::NumFaces];
	std::shared_ptr<IOE::Renderer::IOETexture> m_pDepthTarget;
	IOE::Maths::IOECamera m_arrCameras[IOE::Renderer::ECubeFace::NumFaces];
	IOE::Renderer::IOEVisibilityBounds
		m_arrVisibilityBounds[IOE::Renderer::ECubeFace::NumFaces];

	IOE::Renderer::IOEBlendState m_tDefault;
	IOE::Renderer::IOEBlendState m_tNoBlend;

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pBuildShadowmapMaterial;
};

//////////////////////////////////////////////////////////////////////////

class DualParabaloidShadowMap
{
	static const int32_t ms_nShadowMapSize = 1024;

	float m_fShadowMapSize;

public:
	DualParabaloidShadowMap(float fResolution = (float)ms_nShadowMapSize);

	void Initialise();

	void PreRender(IOE::Renderer::IOERPI *RPI);
	void Render(IOE::Renderer::IOERPI *RPI);
	void PostRender(IOE::Renderer::IOERPI *RPI);

	void SetLightPosition(const IOE::Maths::IOEVector &vLightPos);

	std::shared_ptr<IOE::Renderer::IOETexture> GetShadowMap()
	{
		return m_pShaderResources;
	}

	const IOE::Maths::IOECamera &GetCamera(std::uint32_t uDir) const
	{
		return m_arrCamera[uDir];
	}

private:
	std::shared_ptr<IOE::Renderer::IOETexture> m_pShadowMap;
	std::shared_ptr<IOE::Renderer::IOETexture> m_pDepthMap;

	std::shared_ptr<IOE::Renderer::IOETexture> m_pShaderResources;

	// Dual paraboloid shadow mapping is done by distorting the scene onto 2
	// paraboloid surfaces.
	// This distortion allows us to only have to think about 2 shadow maps
	// rather than 6 at the cost
	// of a slight distortion. We may be able to alleviate some distortion
	// caused by the rasterizer
	// by using a geometry buffer to create new 3D verts rather than relying on
	// the default interpolation.
	std::shared_ptr<IOE::Renderer::IOETexture> m_arrRenderTargets[2];
	IOE::Maths::IOECamera m_arrCamera[2];
	IOE::Renderer::IOEVisibilityBounds m_arrVisibilityBounds[2];

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pBuildShadowmapMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pCopyBufferMaterial[2];

	IOE::Renderer::IOEBlendState m_tDefault;
	IOE::Renderer::IOEBlendState m_tNoBlend;

	IOE::Renderer::IOERasterizerState m_tBackFaceCulling;
	IOE::Renderer::IOERasterizerState m_tFrontFaceCulling;
};