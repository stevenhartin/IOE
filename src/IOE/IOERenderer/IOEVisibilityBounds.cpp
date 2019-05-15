#include <IOE/IOEMaths/AABB.h>
#include <IOE/IOEMaths/Vector.h>

#include <IOE/IOERenderer/IOEModelManager.h>

#include "IOEVisibilityBounds.h"

#include "DirectXCollision.h"

//////////////////////////////////////////////////////////////////////////

using std::uint32_t;
using namespace IOE;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	static DirectX::BoundingBox GetBoxFromAABB(const Maths::IOEAABB &rAABB)
	{
		const Maths::IOEVector &rvMinBounds(rAABB.m_vBounds[0]);
		const Maths::IOEVector &rvMaxBounds(rAABB.m_vBounds[1]);
		Maths::IOEVector vHalfExtents((rvMaxBounds - rvMinBounds) * 0.5f);
		Maths::IOEVector vCentre(rvMinBounds + vHalfExtents);

		XMFLOAT3 arrHalfExtents;
		vHalfExtents.Get(arrHalfExtents.x, arrHalfExtents.y, arrHalfExtents.z);

		XMFLOAT3 arrCentre;
		vCentre.Get(arrCentre.x, arrCentre.y, arrCentre.z);

		DirectX::BoundingBox tBox(arrCentre, arrHalfExtents);

		return tBox;
	}

	//////////////////////////////////////////////////////////////////////////

	IOEVisibilityBounds::IOEVisibilityBounds()
		: m_uNumLinks(0)
		, m_uNumModels(0)
		, m_bTakeIntoAccountLookAt(false)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEVisibilityBounds::UpdateVisibility(IOECamera &rCamera)
	{
		m_uNumLinks  = 0;
		m_uNumModels = 0;

		DirectX::BoundingFrustum tFrustum;
		DirectX::BoundingFrustum::CreateFromMatrix(
			tFrustum, rCamera.GetProjectionMatrix().GetRaw());
		// tFrustum.Transform(tFrustum,
		// rCamera.GetTransform().ToMatrix().GetRaw());
		if (m_bTakeIntoAccountLookAt)
		{
			IOEVector vLookAt(rCamera.GetLookAt() -
							  rCamera.GetTransform().GetTranslation());
			IOEVector vUp(rCamera.GetUpVector());
			vLookAt.NormaliseSet();
			IOEVector vSide(vUp.Cross(vLookAt));
			IOEMatrix mRot(vSide, vUp, vLookAt, Maths::g_vIdentity);
			tFrustum.Transform(tFrustum, mRot.GetRaw());
			tFrustum.Transform(
				tFrustum,
				IOEMatrix::Translation(rCamera.GetTransform().GetTranslation())
					.GetRaw());
		}
		else
		{
			tFrustum.Transform(tFrustum,
							   rCamera.GetTransform().ToMatrix().GetRaw());
		}

		// tFrustum.Transform(tFrustum,
		// rCamera.GetViewMatrix().ToMatrix().GetRaw());

		for (int32_t uIdx(0);
			 uIdx < IOEModelManager::GetSingletonPtr()->GetNumModels(); ++uIdx)
		{
			IOEModel *pModel(
				IOEModelManager::GetSingletonPtr()->GetModel(uIdx));
			IOE_ASSERT(pModel != nullptr, "Model not found");

			if (pModel != nullptr)
			{
				DirectX::BoundingBox tBox(GetBoxFromAABB(pModel->GetAABB()));

				if (!tFrustum.Intersects(tBox) && !tFrustum.Contains(tBox))
				{
					// Entire model is not contained, instantly reject all sub
					// objects
					continue;
				}

				std::int32_t nModelIdx(FindModelIndex(pModel));
				if (nModelIdx == -1)
				{
					nModelIdx = static_cast<std::int32_t>(m_uNumModels);
					m_arrModels[m_uNumModels].pModel = pModel;
					m_arrModels[m_uNumModels].pHead =
						m_arrModels[m_uNumModels].pTail = nullptr;
					++m_uNumModels;
				}
				IOEVisbilityModel &rVisibilityModel(m_arrModels[nModelIdx]);

				// TODO: Put some accelerated BSP method here to speed up
				// searching
				for (int32_t uMeshIdx(0); uMeshIdx < pModel->GetNumSubsets();
					 ++uMeshIdx)
				{
					MeshIndex &rMeshIndex(pModel->GetSubset(uMeshIdx));

					tBox = GetBoxFromAABB(rMeshIndex.m_tAABB);

					if (tFrustum.Intersects(tBox))
					{
						m_arrVisibleLinks[m_uNumLinks].pMeshIndex =
							&rMeshIndex;
						m_arrVisibleLinks[m_uNumLinks].pNext = nullptr;

						if (rVisibilityModel.pTail != nullptr)
						{
							rVisibilityModel.pTail->pNext =
								&m_arrVisibleLinks[m_uNumLinks];
						}
						rVisibilityModel.pTail =
							&m_arrVisibleLinks[m_uNumLinks];
						if (rVisibilityModel.pHead == nullptr)
						{
							rVisibilityModel.pHead =
								&m_arrVisibleLinks[m_uNumLinks];
						}

						++m_uNumLinks;
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void IOEVisibilityBounds::UpdateVisibility(const IOEVector &vOrigin,
											   const IOEVector &vNormal)
	{
		m_uNumLinks  = 0;
		m_uNumModels = 0;

		for (int32_t uIdx(0);
			 uIdx < IOEModelManager::GetSingletonPtr()->GetNumModels(); ++uIdx)
		{
			IOEModel *pModel(
				IOEModelManager::GetSingletonPtr()->GetModel(uIdx));
			IOE_ASSERT(pModel != nullptr, "Model not found");

			if (pModel != nullptr)
			{
				if (!IsAnyInFront(pModel->GetAABB(), vOrigin, vNormal))
				{
					// Entire model is not contained, instantly reject all sub
					// objects
					continue;
				}

				std::int32_t nModelIdx(FindModelIndex(pModel));
				if (nModelIdx == -1)
				{
					nModelIdx = static_cast<std::int32_t>(m_uNumModels);
					m_arrModels[m_uNumModels].pModel = pModel;
					m_arrModels[m_uNumModels].pHead =
						m_arrModels[m_uNumModels].pTail = nullptr;
					++m_uNumModels;
				}
				IOEVisbilityModel &rVisibilityModel(m_arrModels[nModelIdx]);

				// TODO: Put some accelerated BSP method here to speed up
				// searching
				for (int32_t uMeshIdx(0); uMeshIdx < pModel->GetNumSubsets();
					 ++uMeshIdx)
				{
					MeshIndex &rMeshIndex(pModel->GetSubset(uMeshIdx));

					if (IsAnyInFront(rMeshIndex.m_tAABB, vOrigin, vNormal))
					{
						m_arrVisibleLinks[m_uNumLinks].pMeshIndex =
							&rMeshIndex;
						m_arrVisibleLinks[m_uNumLinks].pNext = nullptr;

						if (rVisibilityModel.pTail != nullptr)
						{
							rVisibilityModel.pTail->pNext =
								&m_arrVisibleLinks[m_uNumLinks];
						}
						rVisibilityModel.pTail =
							&m_arrVisibleLinks[m_uNumLinks];
						if (rVisibilityModel.pHead == nullptr)
						{
							rVisibilityModel.pHead =
								&m_arrVisibleLinks[m_uNumLinks];
						}

						++m_uNumLinks;
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	std::int32_t IOEVisibilityBounds::FindModelIndex(IOEModel *pModel) const
	{
		for (uint32_t uIdx(0); uIdx < m_uNumModels; ++uIdx)
		{
			if (m_arrModels[uIdx].pModel == pModel)
			{
				return static_cast<std::int32_t>(uIdx);
			}
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Renderer
} // namespace IOE