#include <fbx/fbxsdk.h>
#include <fbx/fbxsdk/fileio/fbxiosettingspath.h>

#include <IOE/IOECore/IOEEngine.h>
#include <IOE/IOEExceptions/IOEException_Enum.h>
#include <IOE/IOEExceptions/IOEException_Renderer.h>
#include <IOE/IOEApplication/IOEApplication.h>

#include "IOEModelManager.h"
#include "IOEModel.h"
#include "IOEVisibilityBounds.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	template <typename T>
	inline void ReleaseFBX(T *&pObject)
	{
		if (pObject != nullptr)
		{
			pObject->Destroy();
			pObject = nullptr;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	IOEModelManager::IOEModelManager()
		: IOEManager("IOEModelManager")
		, m_pManager(nullptr)
		, m_pGeometryConverter(nullptr)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModelManager::OnManagerPreInit()
	{
		m_pManager = FbxManager::Create();
		if (m_pManager == nullptr)
		{
			THROW_IOE_RENDERER_EXCEPTION("Failed to create FBX Manager",
										 E_ERROR_RENDERER_UNKNOWN);
		}
		// Create the geometry converter
		m_pGeometryConverter = new FbxGeometryConverter(m_pManager);

		// Create an IOSettings object. This object holds all import/export
		// settings.
		FbxIOSettings *ios = FbxIOSettings::Create(m_pManager, IOSROOT);
		m_pManager->SetIOSettings(ios);

		// Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		m_pManager->LoadPluginsDirectory(lPath.Buffer());

		// Get the file version number generate by the FBX SDK.
		FbxManager::GetFileFormatVersion(m_nSDKMajor, m_nSDKMinor,
										 m_nSDKRevision);

		IOE::Core::g_pEngine->GetApplication()->Logf(
			"FBX file format version for this FBX SDK is %d.%d.%d\n",
			m_nSDKMajor, m_nSDKMinor, m_nSDKRevision);
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModelManager::OnDestroy()
	{
		delete m_pGeometryConverter;
		m_pGeometryConverter = nullptr;

		ReleaseFBX(m_pManager);
	}

	//////////////////////////////////////////////////////////////////////////}

	shared_ptr<IOEModel>
	IOEModelManager::LoadModel(const std::wstring &szPath,
							   const std::string &szPassword /*=""*/)
	{
		auto pPtr = std::make_shared<IOEModel>(szPath, szPassword);
		pPtr->Load(this);
		m_arrModels.push_back(pPtr);
		return pPtr;
	}

	//////////////////////////////////////////////////////////////////////////}

	bool IOEModelManager::IsValidFileVersion(FbxImporter *pImporter) const
	{
		int nFileMajor, nFileMinor, nFileRevision;
		pImporter->GetFileVersion(nFileMajor, nFileMinor, nFileRevision);

		// Only care about major revisions for now
		return nFileMajor == m_nSDKMajor;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModelManager::RenderAllModels(IOE::Renderer::IOERPI *pRPI,
										  bool bSetShaderParams /*=true*/)
	{
		for (auto pModel : m_arrToRender)
		{
			pModel.lock()->OnRender(pRPI, bSetShaderParams);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModelManager::RenderAllModels(IOE::Renderer::IOERPI *pRPI,
										  IOEVisibilityBounds &rBounds,
										  bool bSetShaderParams /*=true*/)
	{
		for (std::uint32_t uIdx(0); uIdx < rBounds.GetNumModels(); ++uIdx)
		{
			IOEVisbilityModel &rModel(rBounds.GetModel(uIdx));
			rModel.pModel->OnRender(pRPI, rModel.pHead, bSetShaderParams);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEModelManager::OnUpdate(IOE::Core::IOETimeDelta TimeDelta)
	{
		/*for (auto & rModel : m_arrModels)
		{
			rModel->OnDebugDraw(*IOERPI::GetSingletonPtr());
		}*/
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE