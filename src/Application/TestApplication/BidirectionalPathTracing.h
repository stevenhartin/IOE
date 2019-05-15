#pragma once

#include <memory>

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOERenderer/IOEBuffer.h>
#include <IOE/IOERenderer/IOEVisibilityBounds.h>

#define ENABLE_BIDIRECTIONAL_PATH_TRACER_DUAL_PARABALOID_SHADOW_MAPS 1

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

struct DebugPerPixelLinkedListVertex
{
	DebugPerPixelLinkedListVertex();

	DebugPerPixelLinkedListVertex(const IOE::Maths::IOEVector &vPos_);

	void Set(const IOE::Maths::IOEVector &vPos_);

	IOE::Maths::IOEVector vPos;
};

//////////////////////////////////////////////////////////////////////////

class BidirectionalPathTracer
{
public:
	// Phi = Longitude (Num slices)
	// Theta = Lattitude (Num stacks)
	static const std::int32_t ms_nNumSampleThetaDirs = 4;
	static const std::int32_t ms_nNumSamplePhiDirs   = 5;

	static const std::int32_t ms_nNumSamples =
		ms_nNumSamplePhiDirs * ms_nNumSampleThetaDirs;
	static const float ms_fVPLResolution;

public:
	template <int TNumFaces>
	struct RenderableCubemapTexture
	{
		static const int ms_nNumFaces = TNumFaces;
		void Initialise(float fResolution,
						IOE::Renderer::ETextureFormat eTextureFormat,
						IOE::Renderer::ETextureFormat eResourceFormat,
						IOE::Renderer::ETextureFormat eRenderTargetFormat,
						bool bDepth = false);

		std::shared_ptr<IOE::Renderer::IOETexture> pTexture;
		std::shared_ptr<IOE::Renderer::IOETexture> pResource;

		std::shared_ptr<IOE::Renderer::IOETexture> arrRenderTargets[TNumFaces];
	};

	template <int TNumFaces, int TNumIndices>
	struct RenderableCubemapTextureArray
	{
		static const int ms_nNumFaces = TNumFaces;
		void Initialise(float fResolution,
						IOE::Renderer::ETextureFormat eTextureFormat,
						IOE::Renderer::ETextureFormat eResourceFormat,
						IOE::Renderer::ETextureFormat eRenderTargetFormat,
						bool bDepth = false);

		std::shared_ptr<IOE::Renderer::IOETexture> pTexture;
		std::shared_ptr<IOE::Renderer::IOETexture> pResourceAsArray;
		std::shared_ptr<IOE::Renderer::IOETexture> pResource[TNumIndices];

		std::shared_ptr<IOE::Renderer::IOETexture>
			arrRenderTargets[TNumIndices][TNumFaces];
	};

public:
	BidirectionalPathTracer();
	void PreRender(IOE::Renderer::IOERPI *RPI);
	void Render(IOE::Renderer::IOERPI *RPI);
	void PostRender(IOE::Renderer::IOERPI *RPI);
	void FinishRender(IOE::Renderer::IOERPI *RPI);

	void Initialise();

	void SetLightSettings(const IOEVector &vLightPos, float fIntensity);

	IOE::Renderer::IOETexture *GetHeadBuffer()
	{
		return m_pLinkedListHeadBuffer[2].get();
	}
	IOE::Renderer::IOETexture *GetPixelLinkedListBuffer()
	{
		return m_pPixelBuffer[2].get();
	}
	IOE::Renderer::IOETexture *
	GetRayBundleCameraLocationBuffer()
	{
		return m_pRayBundleCameraLocationBuffer.get();
	}
	IOE::Renderer::IOETexture *GetVPLSampleDataBuffer()
	{
		return m_pVPLSampleDataBuffer[2].get();
	}
	IOE::Renderer::IOETexture *GetVPLShadowMaps()
	{
		return m_arrVPLs[1].pResourceAsArray.get();
	}
	IOE::Maths::IOECamera &GetRayBundleCamera(std::int32_t nIdx)
	{
		return m_arrCamerasGlobalRayBundles[nIdx];
	}

	IOE::Maths::IOECamera GetVPLCamera();

private:
	void CreateReflectiveShadowMap(IOE::Renderer::IOERPI *RPI);
	void CreateVPLSampleData(IOE::Renderer::IOERPI *RPI);
	void GenerateVPLs(IOE::Renderer::IOERPI *RPI);
	void CreatePerPixelLinkedList(IOE::Renderer::IOERPI *RPI);

private:
	void DebugDrawScene(IOE::Renderer::IOERPI *RPI);

private:
	struct ReflectiveShadowMap
	{
		static const int32_t ms_nShadowMapResolution = 256;

		ReflectiveShadowMap();

		void Initialise();

		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tPositionMap;
		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tDepthMap;
		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tNormalMap;
		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tFluxMap;
		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tDiffuseMap;
		RenderableCubemapTexture<(int)IOE::Renderer::ECubeFace::NumFaces>
			tRoughnessMap;
	} m_tShadowMap;

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER_DUAL_PARABALOID_SHADOW_MAPS == 1)
	RenderableCubemapTextureArray<1, ms_nNumSamples> m_arrVPLs[2];
#else
	RenderableCubemapTextureArray<(int)IOE::Renderer::ECubeFace::NumFaces,
								  ms_nNumSamples> m_arrVPLs[2];
#endif

	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pBuildReflectiveShadowMapMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pGenerateVPLMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pGeneratePerPixelLinkedListMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pClearPerPixelLinkedListMaterial;

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pComputeRadianceMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pComputeVPLSampleDataMaterial;

	std::shared_ptr<IOE::Renderer::IOEMaterial>
		m_pDebugPerPixelLinkedListMaterial;

private:
	// [0] = Texture
	// [1] = UAV View
	// [2] = Shader Resource View
	std::shared_ptr<IOE::Renderer::IOETexture> m_pVPLSampleDataBuffer[3];
	std::shared_ptr<IOE::Renderer::IOETexture> m_pLinkedListHeadBuffer[3];
	std::shared_ptr<IOE::Renderer::IOETexture> m_pPixelBuffer[3];
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_pRayBundleCameraLocationBuffer;

	std::shared_ptr<IOE::Renderer::IOETexture>
		m_pGlobalRayBundleRenderTarget[3];
	std::shared_ptr<IOE::Renderer::IOETexture>
		m_pGlobalRayBundleDepthTarget[3];

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pCopyBufferMaterial[2];

	IOE::Maths::IOECamera m_arrCamerasGlobalRayBundles[ms_nNumSamples];
	IOE::Maths::IOEVector m_arrDirections[ms_nNumSamples];

public:
	float m_arrRayBundleLocations[ms_nNumSamples][24];

private:
	IOE::Renderer::IOEBlendState m_tNoColourWrite;
	IOE::Renderer::IOEBlendState m_tDefaultBlend;

	IOE::Renderer::IOERasterizerState m_tBackFaceCulling;
	IOE::Renderer::IOERasterizerState m_tNoCulling;

	IOE::Maths::IOECamera
		m_arrCubemapCameras[IOE::Renderer::ECubeFace::NumFaces];
	IOE::Maths::IOECamera
		m_arrDefaultAxesCameras[IOE::Renderer::ECubeFace::NumFaces];
	IOE::Renderer::IOEVisibilityBounds
		m_arrCubemapVisibility[IOE::Renderer::ECubeFace::NumFaces];
	IOE::Maths::IOECamera m_tForwardCamera;

	IOE::Renderer::IOEBuffer<DebugPerPixelLinkedListVertex> m_tVertexBuffer;

	IOE::Maths::IOEVector m_vCameraPosition;
	IOE::Maths::IOEVector m_vCameraNormal;
};

//////////////////////////////////////////////////////////////////////////