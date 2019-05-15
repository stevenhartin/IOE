#pragma once

#include <memory>
#include <string>
#include <sstream>

#include <IOE/IOERenderer/IOERPI_Defines.h>
#if IOE_RPI == DX11
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "DXGI.lib")
#elif IOE_RPI == DX12
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "DXGI.lib")
#endif

#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEApplication/IOEApplication.h>
#include <IOE/IOERenderer/IOEBuffer.h>
#include <IOE/IOERenderer/IOEVisibilityBounds.h>

#include "Axis.h"
#include "HUD.h"
#include "BidirectionalPathTracing.h"
#include "VoxelConeTracing.h"
#include "ShadowMapping.h"

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

// Enable/Disable scene shadow mapping
#define ENABLE_SHADOW_MAPPING 1

// Only one of these should be defined at a time, this demo does not support
// hot swapping the methods at the moment. They have to be compiled in.
#define ENABLE_VOXEL_CONE_TRACING 0
#define ENABLE_BIDIRECTIONAL_PATH_TRACER 1

//////////////////////////////////////////////////////////////////////////

enum class ECameraMode
{
	Camera,
	Light
};

//////////////////////////////////////////////////////////////////////////

enum class EDebugDrawMode
{
	None,

	DrawVoxelScene,

	DrawPerPixelLinkedList,

	DrawCameraOrientation,

	DrawStencilBuffer,

	DrawRSMPosition,
	DrawRSMNormal,
	DrawRSMFlux,
	DrawRSMDepth,
	DrawRSMRoughness,
	DrawRSMDiffuse,

	DrawVPLFlux,
	DrawVPLDepth,

	NumDebugDrawModes
};

FORCEINLINE bool IsVoxelDebugDrawMode(EDebugDrawMode eMode)
{
	switch (eMode)
	{
	case EDebugDrawMode::DrawVoxelScene:
		return true;
	}
	return false;
}

FORCEINLINE bool IsBidirectionalPathTracingDebugDraw(EDebugDrawMode eMode)
{
	switch (eMode)
	{
	case EDebugDrawMode::DrawCameraOrientation:
	case EDebugDrawMode::DrawStencilBuffer:
	case EDebugDrawMode::DrawRSMPosition:
	case EDebugDrawMode::DrawRSMNormal:
	case EDebugDrawMode::DrawRSMFlux:
	case EDebugDrawMode::DrawRSMDepth:
	case EDebugDrawMode::DrawRSMRoughness:
	case EDebugDrawMode::DrawRSMDiffuse:
	case EDebugDrawMode::DrawVPLFlux:
	case EDebugDrawMode::DrawVPLDepth:
		return true;
	}
	return false;
}

FORCEINLINE EDebugDrawMode GetNextDebugDrawMode(EDebugDrawMode eCurrentMode)
{
	EDebugDrawMode eTemp = (EDebugDrawMode)(
		((int)(eCurrentMode) + 1) % (int)(EDebugDrawMode::NumDebugDrawModes));
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 0)
	if (IsBidirectionalPathTracingDebugDraw(eTemp))
	{
		return GetNextDebugDrawMode(eTemp);
	}
#endif
#if (ENABLE_VOXEL_CONE_TRACING == 0)
	if (IsVoxelDebugDrawMode(eTemp))
	{
		return GetNextDebugDrawMode(eTemp);
	}
#endif
	return eTemp;
}

FORCEINLINE EDebugDrawMode
GetPreviousDebugDrawMode(EDebugDrawMode eCurrentMode)
{
	int nMode((int)eCurrentMode - 1);
	if (nMode < 0)
	{
		nMode += (int)EDebugDrawMode::NumDebugDrawModes;
	}
	EDebugDrawMode eTemp = (EDebugDrawMode)(nMode);
#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 0)
	if (IsBidirectionalPathTracingDebugDraw(eTemp))
	{
		return GetPreviousDebugDrawMode(eTemp);
	}
#endif
#if (ENABLE_VOXEL_CONE_TRACING == 0)
	if (IsVoxelDebugDrawMode(eTemp))
	{
		return GetPreviousDebugDrawMode(eTemp);
	}
#endif
	return eTemp;
}

FORCEINLINE const char *GetDebugDrawModeName(EDebugDrawMode eDrawMode)
{
	switch (eDrawMode)
	{
	case EDebugDrawMode::None:
		return "None";
	case EDebugDrawMode::DrawCameraOrientation:
		return "Camera Orientation";
	case EDebugDrawMode::DrawStencilBuffer:
		return "Stencil Buffer";
	case EDebugDrawMode::DrawRSMPosition:
		return "Reflective Shadow Map - Position";
	case EDebugDrawMode::DrawRSMNormal:
		return "Reflective Shadow Map - Normal";
	case EDebugDrawMode::DrawRSMFlux:
		return "Reflective Shadow Map - Flux";
	case EDebugDrawMode::DrawRSMDepth:
		return "Reflective Shadow Map - Depth";
	case EDebugDrawMode::DrawRSMRoughness:
		return "Reflective Shadow Map - Roughness";
	case EDebugDrawMode::DrawRSMDiffuse:
		return "Reflective Shadow Map - DIffuse";
	case EDebugDrawMode::DrawVPLFlux:
		return "VPL - Flux";
	case EDebugDrawMode::DrawVPLDepth:
		return "VPL - Depth";
	case EDebugDrawMode::DrawVoxelScene:
		return "Voxel Scene";
	case EDebugDrawMode::DrawPerPixelLinkedList:
		return "Per Pixel Linked List";
	default:
	case EDebugDrawMode::NumDebugDrawModes:
		return "Unknown Debug Draw Mode";
	}
	return "Unknown Debug Draw Mode";
}

//////////////////////////////////////////////////////////////////////////

struct SerialisedData
{
	struct _InternalData
	{
		float LightPosition[3];
		float LightColour[4];
		float LightSpecular;
		float LightIntensity;
		float LightRoughness;
		float LightRadius;

		IOE::Maths::IOECamera::CCameraProperties CameraProperties;
		IOE::Maths::IOEMatrix CameraTransform;
		float CameraLookAt[3];
	};

	void AddNewData();
	void SetSettings(std::int32_t nIdx);
	void RemoveIndex(std::int32_t nIdx);

	std::uint32_t uNumElements;
	std::vector<_InternalData> m_arrData;
};

//////////////////////////////////////////////////////////////////////////

DECLARE_MANAGER_CHILD(CTestApplication, Application)
{
public:
	CTestApplication(const IOE::Application::C_CommandLine &rCommandLine);

	virtual void CreateApplication() override final;

	virtual void OnManagerPostInit() override final;

	virtual void OnUpdate(IOE::Core::IOETimeDelta tTimeDelta) override final;
	virtual void OnRender(IOE::Renderer::IOERPI *pPtr)
		override final;
	virtual void OnResize(std::uint32_t uWidth, std::uint32_t uHeight)
		override final;

	FORCEINLINE std::shared_ptr<IOE::Renderer::IOEMaterial> GetActiveMaterial()
	{
		return m_pActiveMaterial;
	}
	FORCEINLINE IOE::Renderer::IOEModel *GetMainModel()
	{
		return m_pSponza.get();
	}

	IOE::Renderer::IOERasterizerState &GetDefaultRasterizerState()
	{
		return m_tDefaultRasterizerState;
	}
	IOE::Renderer::IOERasterizerState &GetNoClipRasterizerState()
	{
		return m_tNoClipRasterizerState;
	}

	void CopyFullscreenTarget(
		IOE::Renderer::IOERPI *pRPI,
		IOE::Renderer::IOETexture *pSrcTexture,
		IOE::Renderer::IOEMaterial *pSrcMaterial = nullptr,
		IOE::Renderer::ECubeFace eFace = IOE::Renderer::ECubeFace::All,
		float fLightModifier = 1.0f);

	FORCEINLINE EDebugDrawMode GetDebugDrawMode() const
	{
		return m_eDebugDrawMode;
	}

	FORCEINLINE const IOE::Maths::IOEVector &GetLightPosition() const
	{
		return m_vLightPos;
	}
	FORCEINLINE void SetLightPosition(
		const IOE::Maths::IOEVector &vLightPosition);

	FORCEINLINE const IOE::Maths::IOEVector &GetLightColour() const
	{
		return m_vLightColour;
	}
	FORCEINLINE void SetLightColour(const IOE::Maths::IOEVector &vLightColour);

	FORCEINLINE float GetLightIntensity() const
	{
		return m_fIntensity;
	}
	FORCEINLINE void SetLightIntensity(float fIntensity)
	{
		m_fIntensity = fIntensity;
		UpdateIntensity();
	}

	FORCEINLINE float GetRoughness() const
	{
		return m_fRoughness;
	}
	FORCEINLINE void SetRoughness(float fRoughness)
	{
		m_fRoughness = fRoughness;
		UpdateIntensity();
	}

	FORCEINLINE float GetSpecular() const
	{
		return m_fLightSpecular;
	}
	FORCEINLINE void SetSpecular(float fSpecular)
	{
		m_fLightSpecular = fSpecular;
		UpdateIntensity();
	}

	FORCEINLINE float GetLightRadius() const
	{
		return m_fLightRadius;
	}
	FORCEINLINE void SetLightRadius(float fRadius)
	{
		m_fLightRadius = fRadius;
		UpdateIntensity();
	}

	FORCEINLINE virtual IOE::Application::IOEWindowOptions GetWindowOptions()
		override final
	{
		IOE::Application::IOEWindowOptions tWindowOptions;
		tWindowOptions.bCreateConsole = 0;
		return tWindowOptions;
	}

	void SetCameraSettings(IOE::Maths::IOECamera & rCamera);

	void HandleInput(IOE::Core::IOETimeDelta tTimeDelta);
	void HandleInput_Debug(IOE::Core::IOETimeDelta tTimeDelta);
	void DebugDraw();

	void SerialiseData();
	void UnserialiseData();

private:
	void UpdateIntensity();
	void SetupFullscreenBuffers(IOE::Renderer::IOERPI * pRPI);

private:
	float m_fIntensity;
	float m_fRoughness;

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pDiffuseMaterial;
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pBumpMappingMaterial;
	// [0] = GI and Direct Lighting
	// [1] = Just Indirect Lighting
	// [2] = Just Direct Lighting
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pPhysicalMaterial[3];
	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pCopyBufferMaterial[7];

	std::shared_ptr<IOE::Renderer::IOEMaterial> m_pActiveMaterial;

	std::shared_ptr<IOE::Renderer::IOEModel> m_pSponza;

	IOE::Maths::IOECamera m_tOrthographicCamera;

#if (ENABLE_VOXEL_CONE_TRACING == 1)
	VoxelConeTracer m_tVoxelConeTracer;
#endif

#if (ENABLE_BIDIRECTIONAL_PATH_TRACER == 1)
	BidirectionalPathTracer m_tBidirectionalPathTracer;
#endif

#if (ENABLE_SHADOW_MAPPING == 1)
#if (DUAL_PARABALOID_SHADOW_MAPPING == 1)
	DualParabaloidShadowMap m_tShadowMap;
#else
	CubeShadowMap m_tShadowMap;
#endif // DUAL_PARABALOID_SHADOW_MAPPING == 1
#endif

	IOE::Renderer::IOEBuffer<DebugOctreeVertex> m_tFullscreenBuffer;

	IOE::Renderer::IOERasterizerState m_tNoClipRasterizerState;
	IOE::Renderer::IOERasterizerState m_tDefaultRasterizerState;

	IOE::Maths::IOEVector m_vLightPos;
	IOE::Maths::IOEVector m_vLightColour;
	float m_fLightSpecular;
	float m_fLightRadius;

	IOE::Maths::IOEVector m_vCameraPosition;
	IOE::Maths::IOEQuaternion m_qCameraRotation;

	std::shared_ptr<IOE::Renderer::IOESamplerState> m_pClampSampler;
	std::shared_ptr<IOE::Renderer::IOESamplerState> m_pDefaultSampler;

	EDebugDrawMode m_eDebugDrawMode;

	IOE::Renderer::IOEVisibilityBounds m_tCameraVisibilityCheck;
	bool m_bCameraDirty;

	ECameraMode m_eCameraMode;

	double m_dRenderTime;
	int32_t m_nNumnRenders;

	float m_fLastRender;

	std::string m_szDebugMode;
	std::string m_szLightingMode;
	int m_nLightingMode;

	int m_nCurrentSerialisedDataIdx;

	std::shared_ptr<IOE::Renderer::IOEFont> m_pFont;

	HUD m_tHUD;

	SerialisedData m_tSerialisedData;
};

//////////////////////////////////////////////////////////////////////////
