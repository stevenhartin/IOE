#pragma once

#include "vector.h"
#include "IOERay.h"
#include "Matrix.h"

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class IOEAABB
	{
	public:
		IOEAABB()
		{
			m_vBounds[0] = g_vIdentity;
			m_vBounds[1] = g_vIdentity;
		}

		IOEAABB(const IOEVector &vMin, const IOEVector &vMax)
		{
			Set(vMin, vMax);
		}

		void Set(const IOEVector &vMin, const IOEVector &vMax)
		{
			m_vBounds[0] = vMin;
			m_vBounds[1] = vMax;
		}

		IOEAABB Transform(IOEMatrix &rMat)
		{
			IOEAABB tNew;

			// Get the 8 points of the AABB
			float arrMin[3], arrMax[3];

			IOEVector vCentre(m_vBounds[0] +
							  ((m_vBounds[1] - m_vBounds[0]) * 0.5f));

			IOEMatrix rOffsetTransform = IOEMatrix::Translation(-vCentre) *
				rMat * IOEMatrix::Translation(vCentre);

			m_vBounds[0].Get(arrMin[0], arrMin[1], arrMin[2]);
			m_vBounds[1].Get(arrMax[0], arrMax[1], arrMax[2]);

			IOEVector arrVertices[] = {
				IOEVector(arrMin[0], arrMin[1], arrMin[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMin[0], arrMin[1], arrMax[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMin[0], arrMax[1], arrMin[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMin[0], arrMax[1], arrMax[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMax[0], arrMin[1], arrMin[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMax[0], arrMin[1], arrMax[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMax[0], arrMax[1], arrMin[2])
					.Multiply(rOffsetTransform),
				IOEVector(arrMax[0], arrMax[1], arrMax[2])
					.Multiply(rOffsetTransform),
			};
			IOEVector vMin = arrVertices[0];
			IOEVector vMax = arrVertices[0];
			for (int32_t nIdx(0); nIdx < 8; ++nIdx)
			{
				vMin.MinSet(arrVertices[nIdx]);
				vMax.MaxSet(arrVertices[nIdx]);
			}

			tNew.m_vBounds[0] = vMin;
			tNew.m_vBounds[1] = vMax;
			return tNew;
		}

		bool Intersect(const IOERay &ray, float *pfDistance = nullptr) const
		{
			if (pfDistance != nullptr)
				*pfDistance = 0.0f;

			Maths::IOEVector vMinLocal(IOEVector::Multiply(
				(m_vBounds[0] - ray.m_vOrigin), ray.m_vInvNormalisedDir));
			Maths::IOEVector vMaxLocal(IOEVector::Multiply(
				(m_vBounds[1] - ray.m_vOrigin), ray.m_vInvNormalisedDir));

			float tmin = IOEVector::Min(vMinLocal, vMaxLocal).MaxElement();
			float tmax = IOEVector::Max(vMinLocal, vMaxLocal).MinElement();

			// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is
			// behing us
			if (tmax < 0)
			{
				if (pfDistance != nullptr)
					*pfDistance = tmax;
				return false;
			}

			// if tmin > tmax, ray doesn't intersect AABB
			if (tmin > tmax)
			{
				if (pfDistance != nullptr)
					*pfDistance = tmax;
				return false;
			}
			if (pfDistance != nullptr)
				*pfDistance = tmin;
			return true;
		}

		IOEVector m_vBounds[2];
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE