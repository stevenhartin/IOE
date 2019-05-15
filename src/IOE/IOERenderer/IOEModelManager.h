#pragma once

//////////////////////////////////////////////////////////////////////////

#include <vector>
#include <cstdint>

#include <fbxsdk.h>

#include <IOE/IOECore/IOEManager.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEModel;
	class IOERPI;

	using std::shared_ptr;
	using std::weak_ptr;
	using std::vector;

	//////////////////////////////////////////////////////////////////////////

	class IOEModelManager : public IOE::Core::IOEManager<IOEModelManager>
	{
	public:
		IOEModelManager();

		virtual void OnManagerPreInit() override;
		virtual void OnUpdate(IOE::Core::IOETimeDelta TimeDelta) override;
		virtual void OnDestroy() override;

	public:
		shared_ptr<IOEModel> LoadModel(const std::wstring &szPath,
									   const std::string &szPassword = "");

		FbxManager *GetFbxManager()
		{
			return m_pManager;
		}
		const FbxManager *GetFbxManager() const
		{
			return m_pManager;
		}

		FbxGeometryConverter *GetGeometryConverter()
		{
			return m_pGeometryConverter;
		}
		const FbxGeometryConverter *GetGeometryConverter() const
		{
			return m_pGeometryConverter;
		}

		int32_t GetNumModels() const
		{
			return static_cast<int32_t>(m_arrModels.size());
		}
		shared_ptr<IOEModel> GetModelShared(int32_t nIdx)
		{
			return m_arrModels[nIdx];
		}
		IOEModel *GetModel(int32_t nIdx)
		{
			return m_arrModels[nIdx].get();
		}

		void AddToRender(weak_ptr<IOEModel> pModel)
		{
			m_arrToRender.push_back(pModel);
		}

	public:
		void RenderAllModels(IOE::Renderer::IOERPI *pRPI,
							 bool bSetShaderParams = true);
		void RenderAllModels(IOE::Renderer::IOERPI *pRPI,
							 class IOEVisibilityBounds &rBounds,
							 bool bSetShaderParams = true);

	public:
		bool IsValidFileVersion(FbxImporter *pImporter) const;

	private:
		std::int32_t m_nSDKMajor, m_nSDKMinor, m_nSDKRevision;

		class FbxManager *m_pManager;
		class FbxGeometryConverter *m_pGeometryConverter;
		vector<shared_ptr<IOEModel> > m_arrModels;
		vector<weak_ptr<IOEModel> > m_arrToRender;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE