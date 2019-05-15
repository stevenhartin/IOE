#pragma once

//////////////////////////////////////////////////////////////////////////

#include <array>
#include <cstdint>

#include <IOE/IOECore/IOEDefines.h>
#include <IOE/IOERenderer/IOEModel.h>

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Renderer
{

	//////////////////////////////////////////////////////////////////////////

	class IOEModel;

	//////////////////////////////////////////////////////////////////////////

	struct IOEVisibilityLink
	{
		IOEVisibilityLink()
			: pMeshIndex(nullptr)
			, pNext(nullptr)
		{
		}

		MeshIndex *pMeshIndex;
		IOEVisibilityLink *pNext;
	};

	//////////////////////////////////////////////////////////////////////////

	struct IOEVisbilityModel
	{
		IOEVisbilityModel()
			: pModel(nullptr)
			, pHead(nullptr)
			, pTail(nullptr)
		{
		}

		IOEModel *pModel;
		IOEVisibilityLink *pHead;
		IOEVisibilityLink *pTail;
	};

	//////////////////////////////////////////////////////////////////////////

	class IOEVisibilityBounds
	{
		static const std::uint32_t ms_uMaxNumVisibleLinks = 512;
		static const std::uint32_t ms_uNumModels		  = 32;

	public:
		IOEVisibilityBounds();

	public:
		void UpdateVisibility(IOECamera &rCamera);
		void UpdateVisibility(const IOEVector &vOrigin,
							  const IOEVector &vNormal);

		FORCEINLINE std::uint32_t GetNumVisibleMeshes() const
		{
			return m_uNumLinks;
		}
		FORCEINLINE MeshIndex *GetMeshIndex(std::uint32_t uIdx)
		{
			return m_arrVisibleLinks[uIdx].pMeshIndex;
		}

		FORCEINLINE std::uint32_t GetNumModels() const
		{
			return m_uNumModels;
		}
		FORCEINLINE IOEVisbilityModel &GetModel(std::uint32_t uIdx)
		{
			return m_arrModels[uIdx];
		}

		FORCEINLINE void SetTakeIntoAccountLookAt(bool bValue)
		{
			m_bTakeIntoAccountLookAt = bValue;
		}

	private:
		FORCEINLINE bool IsInFront(const IOEVector &vPos,
								   const IOEVector &vOrigin,
								   const IOEVector &vDir) const
		{
			return (vPos - vOrigin).Dot(vDir) >= 0.0f;
		}

		FORCEINLINE bool IsInFront(const IOEVector &vPos,
								   const IOEVector &vDir) const
		{
			return vPos.Dot(vDir) >= 0.0f;
		}

		FORCEINLINE bool IsAnyInFront(const IOEAABB &rAABB,
									  const IOEVector &vOrigin,
									  const IOEVector &vDir) const
		{
			// Test min bound
			IOEVector vMin(rAABB.m_vBounds[0] - vOrigin);
			if (IsInFront(vMin, vDir))
			{
				return true;
			}

			// Test max bound
			IOEVector vMax(rAABB.m_vBounds[1] - vOrigin);
			if (IsInFront(vMax, vDir))
			{
				return true;
			}

			// Test all other corners
			float arrMin[3], arrMax[3];
			vMin.Get(arrMin[0], arrMin[1], arrMin[2]);
			vMax.Get(arrMax[0], arrMax[1], arrMax[2]);

			// 001
			if (IsInFront(IOEVector(arrMin[0], arrMin[1], arrMax[2]), vDir))
			{
				return true;
			}

			// 010
			if (IsInFront(IOEVector(arrMin[0], arrMax[1], arrMin[2]), vDir))
			{
				return true;
			}

			// 011
			if (IsInFront(IOEVector(arrMin[0], arrMax[1], arrMax[2]), vDir))
			{
				return true;
			}

			// 100
			if (IsInFront(IOEVector(arrMax[0], arrMin[1], arrMin[2]), vDir))
			{
				return true;
			}

			// 101
			if (IsInFront(IOEVector(arrMax[0], arrMin[1], arrMax[2]), vDir))
			{
				return true;
			}

			// 110
			if (IsInFront(IOEVector(arrMax[0], arrMax[1], arrMin[2]), vDir))
			{
				return true;
			}

			// No points in front, fully outside of plane, cull
			return false;
		}

		std::int32_t FindModelIndex(IOEModel *pModel) const;

	private:
		std::array<IOEVisibilityLink, ms_uMaxNumVisibleLinks>
			m_arrVisibleLinks;
		std::array<IOEVisbilityModel, ms_uNumModels> m_arrModels;
		std::uint32_t m_uNumLinks;
		std::uint32_t m_uNumModels;
		bool m_bTakeIntoAccountLookAt;
	};

} // namespace Renderer
} // namespace IOE