#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>

#include <fbxsdk.h>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOEMaths/Vector.h>
#include <IOE/IOEMaths/AABB.h>
#include <IOE/IOERenderer/IOEMaterial.h>
#include <IOE/IOERenderer/IOEVertexFormat.h>

#include RPI_INCLUDE(IOEModel_Platform.h)

using std::int32_t;
using std::shared_ptr;
using std::vector;
using namespace IOE::Maths;

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOERPI;
	class IOEModelManager;
	class IOEMaterial;
	class IOEMaterialInstance;
	class IOETexture;

	//////////////////////////////////////////////////////////////////////////

	struct MeshIndex
	{
		MeshIndex(int32_t nStartIdx, int32_t nNumIndices)
			: m_nStartIdx(nStartIdx)
			, m_nNumIndices(nNumIndices)
			, m_pMaterial(nullptr)
		{
		}

		void SetAABB(const IOEVector &vMin, const IOEVector &vMax)
		{
			m_tAABB.Set(vMin, vMax);
		}

		int32_t m_nStartIdx;
		int32_t m_nNumIndices;

		shared_ptr<IOEMaterialInstance> m_pMaterial;
		vector<shared_ptr<IOETexture> > m_arrTextures;
		IOEAABB m_tAABB;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEModel
	{
	public:
		IOEModel(const std::wstring &szPath, const std::string &szPassword);

	public:
		bool Load(IOEModelManager *pManager);

		FORCEINLINE void OnRender(IOERPI *pRPI, bool bSetShaderParams = true)
		{
			IOE_ASSERT(m_pMaterial != nullptr,
					   "Material must be set before model can be rendered");
			m_pMaterial->Set(pRPI, bSetShaderParams);
			m_tPlatform.OnRender(pRPI, bSetShaderParams);
		}
		void OnRender(IOERPI *pRPI, struct IOEVisibilityLink *pHead,
					  bool bSetShaderParams = true)
		{
			IOE_ASSERT(m_pMaterial != nullptr,
					   "Material must be set before model can be rendered");
			m_pMaterial->Set(pRPI, bSetShaderParams);
			m_tPlatform.OnRender(pRPI, pHead, bSetShaderParams);
		}

		void SetMaterial(IOERPI *pRPI, std::shared_ptr<IOE::Renderer::IOEMaterial> pMaterial,
						 bool bSetVariables = true);

		FORCEINLINE std::int32_t GetNumSubsets() const
		{
			return static_cast<std::int32_t>(m_arrSubsets.size());
		}
		FORCEINLINE const MeshIndex &GetSubset(std::int32_t nIdx) const
		{
			return m_arrSubsets[nIdx];
		}
		FORCEINLINE MeshIndex &GetSubset(std::int32_t nIdx)
		{
			return m_arrSubsets[nIdx];
		}

		FORCEINLINE const std::vector<ModelVertex> &GetVertexBuffer() const
		{
			return m_arrVertices;
		}
		FORCEINLINE const std::vector<int32_t> &GetIndexBuffer() const
		{
			return m_arrIndices;
		}

		FORCEINLINE const std::wstring &GetPath() const
		{
			return m_szPath;
		}

		FORCEINLINE void OnDebugDraw(IOERPI *pRPI)
		{
			/*for (auto &rVertex : m_arrVertices)
			{
				rRPI.AddDebugLine(IOEVector(rVertex.m_vPosition),
			IOEVector(rVertex.m_vPosition) + IOEVector(rVertex.m_vNormal),
					Maths::ColourList::Green, Maths::ColourList::Red);
			}*/
			for (auto &rSubset : m_arrSubsets)
			{
				float fXMin, fYMin, fZMin;
				float fXMax, fYMax, fZMax;
				rSubset.m_tAABB.m_vBounds[0].Get(fXMin, fYMin, fZMin);
				rSubset.m_tAABB.m_vBounds[1].Get(fXMax, fYMax, fZMax);

				// Front face
				pRPI->AddDebugLine(IOEVector(fXMin, fYMin, fZMin),
								   IOEVector(fXMin, fYMax, fZMin));
				pRPI->AddDebugLine(IOEVector(fXMin, fYMax, fZMin),
								   IOEVector(fXMax, fYMax, fZMin));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMax, fZMin),
								   IOEVector(fXMax, fYMin, fZMin));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMin, fZMin),
								   IOEVector(fXMin, fYMin, fZMin));

				// Rear face
				pRPI->AddDebugLine(IOEVector(fXMin, fYMin, fZMax),
								   IOEVector(fXMin, fYMax, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMin, fYMax, fZMax),
								   IOEVector(fXMax, fYMax, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMax, fZMax),
								   IOEVector(fXMax, fYMin, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMin, fZMax),
								   IOEVector(fXMin, fYMin, fZMax));

				// Connectors
				pRPI->AddDebugLine(IOEVector(fXMin, fYMin, fZMin),
								   IOEVector(fXMin, fYMin, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMin, fYMax, fZMin),
								   IOEVector(fXMin, fYMax, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMax, fZMin),
								   IOEVector(fXMax, fYMax, fZMax));
				pRPI->AddDebugLine(IOEVector(fXMax, fYMin, fZMin),
								   IOEVector(fXMax, fYMin, fZMax));
			}
		}

		IOEAABB GetAABB()
		{
			IOEAABB tAABB(
				IOEVector(std::numeric_limits<float>::infinity()),   // Min
				IOEVector(-std::numeric_limits<float>::infinity())); // Max

			for (MeshIndex &rIndex : m_arrSubsets)
			{
				tAABB.Set(IOEVector::Min(tAABB.m_vBounds[0],
										 rIndex.m_tAABB.m_vBounds[0]),
						  IOEVector::Max(tAABB.m_vBounds[1],
										 rIndex.m_tAABB.m_vBounds[1]));
			}
			return tAABB;
		}

	private:
		bool LoadScene(IOEModelManager *pManager);
		bool LoadNode(FbxNode *pNode, IOEModelManager *pManager);
		bool LoadGeometry(FbxMesh *pMesh, IOEModelManager *pManager);

	private:
		IOEModel_Platform m_tPlatform;

		std::wstring m_szPath;
		std::string m_szPassword;
		class FbxScene *m_pScene;

		std::shared_ptr<IOE::Renderer::IOEMaterial> m_pMaterial;

		std::vector<ModelVertex> m_arrVertices;
		std::vector<int32_t> m_arrIndices;
		std::vector<MeshIndex> m_arrSubsets;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE