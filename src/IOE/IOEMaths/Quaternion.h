#pragma once

#include <DirectXMath.h>

#include "Vector.h"
#include "Matrix.h"
#include "Math.h"

using namespace DirectX;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	class alignas(16) IOEQuaternion
	{
	private:
		XMVECTOR m_qData;

	public:
		FORCEINLINE IOEQuaternion()
		{
		}

		FORCEINLINE IOEQuaternion(float fX, float fY, float fZ, float fW)
		{
			Load(fX, fY, fZ, fW);
		}

		FORCEINLINE IOEQuaternion(const IOEVector &vDir, float fAngleRad)
		{
			SetFromVectorAndAngle(vDir, fAngleRad);
		}

		FORCEINLINE IOEQuaternion(const IOEVector &vFwd, const IOEVector &vUp)
		{
			IOE_ASSERT(vFwd.IsNormalised() && vUp.IsNormalised(),
				"Forward and up must be normalised");
			IOEVector vRight = IOEVector::Cross(vUp, vFwd);

			const float a[3][3] = {
				{vRight.GetX(), vRight.GetY(), vRight.GetZ() },
				{vUp.GetX(), vUp.GetY(), vUp.GetZ() },
				{vFwd.GetX(), vFwd.GetY(), vFwd.GetZ() },
			};
			float q[4];

			float trace = a[0][0] + a[1][1] + a[2][2];
			if (trace > 0)
			{
				float s = 0.5f / sqrtf(trace + 1.0f);
				q[3] = 0.25f / s;
				q[0] = (a[2][1] - a[1][2]) * s;
				q[1] = (a[0][2] - a[2][0]) * s;
				q[2] = (a[1][0] - a[0][1]) * s;
			}
			else
			{
				if (a[0][0] > a[1][1] && a[0][0] > a[2][2])
				{
					float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
					q[3] = (a[2][1] - a[1][2]) / s;
					q[0] = 0.25f * s;
					q[1] = (a[0][1] + a[1][0]) / s;
					q[2] = (a[0][2] + a[2][0]) / s;
				}
				else if (a[1][1] > a[2][2])
				{
					float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
					q[3] = (a[0][2] - a[2][0]) / s;
					q[0] = (a[0][1] + a[1][0]) / s;
					q[1] = 0.25f * s;
					q[2] = (a[1][2] + a[2][1]) / s;
				}
				else
				{
					float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
					q[3] = (a[1][0] - a[0][1]) / s;
					q[0] = (a[0][2] + a[2][0]) / s;
					q[1] = (a[1][2] + a[2][1]) / s;
					q[2] = 0.25f * s;
				}
			}
			Load(q[0],q[1],q[2],q[3]);
		}

		FORCEINLINE IOEQuaternion(const IOEMatrix &mRotation)
		{
			SetFromRotationMatrix(mRotation);
		}

		FORCEINLINE IOEQuaternion(float fPitch, float fYaw, float fRoll)
		{
			SetFromEuler(fPitch, fYaw, fRoll);
		}

		FORCEINLINE IOEQuaternion(const XMVECTOR &rhs)
			: m_qData(rhs)
		{
		}

		FORCEINLINE IOEQuaternion(XMVECTOR && rhs)
			: m_qData(std::move(rhs))
		{
		}

		FORCEINLINE IOEQuaternion(const IOEQuaternion &rhs)
			: m_qData(rhs.m_qData)
		{
		}

		FORCEINLINE IOEQuaternion(IOEQuaternion && rhs)
			: m_qData(std::move(rhs.m_qData))
		{
		}

		FORCEINLINE IOEQuaternion &operator=(const XMVECTOR &rhs)
		{
			m_qData = rhs;
			return *this;
		}

		FORCEINLINE IOEQuaternion &operator=(XMVECTOR && rhs)
		{
			m_qData = std::move(rhs);
			return *this;
		}

		FORCEINLINE IOEQuaternion &operator=(const IOEQuaternion &rhs)
		{
			m_qData = rhs.m_qData;
			return *this;
		}

		FORCEINLINE IOEQuaternion &operator=(IOEQuaternion && rhs)
		{
			m_qData = std::move(rhs.m_qData);
			return *this;
		}

	public:
		FORCEINLINE bool operator==(const IOEQuaternion &rhs) const
		{
			return XMVector4NearEqual(m_qData, rhs.m_qData, g_XMEpsilon);
		}

		FORCEINLINE bool operator!=(const IOEQuaternion &rhs) const
		{
			return !(operator==(rhs));
		}

	public:
		FORCEINLINE operator CXMVECTOR() const
		{
			return m_qData;
		}

		FORCEINLINE operator XMVECTOR() const
		{
			return m_qData;
		}

		FORCEINLINE const XMVECTOR &GetRaw() const
		{
			return m_qData;
		}

	public:
		FORCEINLINE void SetFromEuler(float fPitch, float fYaw, float fRoll)
		{
			XMFLOAT4 tData(fPitch, fYaw, fRoll, 0.0f);
			XMVECTOR vEulers = XMLoadFloat4(&tData);
			vEulers			 = XMVectorMultiply(
				vEulers,
				XMVectorReplicate(static_cast<float>(IOE_DEG_TO_RAD)));
			m_qData = XMQuaternionRotationRollPitchYawFromVector(vEulers);
		}

		FORCEINLINE void SetFromRotationMatrix(const IOEMatrix &mRotation)
		{
			m_qData = XMQuaternionRotationMatrix(mRotation.GetRaw());
		}

		FORCEINLINE IOEMatrix ToRotationMatrix() const
		{
			float fX, fY, fZ, fW;
			Store(fX, fY, fZ, fW);
			IOEMatrix mResult(1.0f - 2.0f * fY * fY - 2.0f * fZ * fZ,
							  2.0f * fX * fY - 2.0f * fZ * fW,
							  2.0f * fX * fZ + 2.0f * fY * fW, 0.0f,
							  2.0f * fX * fY + 2.0f * fZ * fW,
							  1.0f - 2.0f * fX * fX - 2.0f * fZ * fZ,
							  2.0f * fY * fZ - 2.0f * fX * fW, 0.0f,
							  2.0f * fX * fZ - 2.0f * fY * fW,
							  2.0f * fY * fZ + 2.0f * fX * fW,
							  1.0f - 2.0f * fX * fX - 2.0f * fY * fY, 0.0f,
							  0.0f, 0.0f, 0.0f, 1.0f);
			return mResult;
		}

		FORCEINLINE void SetFromVectorAndAngle(const IOEVector &vAxis,
											   float fAngleRad)
		{
			m_qData = XMQuaternionRotationAxis(vAxis.GetRaw(), fAngleRad);
		}

		FORCEINLINE void GetAxisAndAngle(IOEVector & vAxis, float &fAngle)
		{
			XMVECTOR vTempAxis;
			XMQuaternionToAxisAngle(&vTempAxis, &fAngle, m_qData);
			vAxis = vTempAxis;
		}

		FORCEINLINE float GetPitch() const
		{
			float fX, fY, fZ, fW;
			Store(fX, fY, fZ, fW);

			return RadiansToDegrees(
				ATan2(2.0f * fX * fW - 2.0f * fY * fZ,
					  1.0f - 2.0f * fX * fX - 2.0f * fZ * fZ));
		}

		FORCEINLINE float GetYaw() const
		{
			float fX, fY, fZ, fW;
			Store(fX, fY, fZ, fW);

			return RadiansToDegrees(
				ATan2(2.0f * fY * fW - 2.0f * fX * fZ,
					  1.0f - 2.0f * fY * fY - 2.0f * fZ * fZ));
		}

		FORCEINLINE float GetRoll() const
		{
			float fX, fY, fZ, fW;
			Store(fX, fY, fZ, fW);

			return RadiansToDegrees(ASin(2.0f * (fX * fY + fZ * fW)));
		}

	public:
		FORCEINLINE static IOEQuaternion Slerp(
			const IOEQuaternion &lhs, const IOEQuaternion &rhs, float fDelta)
		{
			IOEQuaternion qNew(lhs);
			qNew.SlerpSet(rhs, fDelta);
			return qNew;
		}

		FORCEINLINE static IOEQuaternion Slerp(const IOEQuaternion &lhs,
											   const IOEQuaternion &rhs,
											   const IOEVector &vDelta)
		{
			IOEQuaternion qNew(lhs);
			qNew.SlerpSet(rhs, vDelta);
			return qNew;
		}

		FORCEINLINE IOEQuaternion Slerp(const IOEQuaternion &rhs, float fDelta)
		{
			IOEQuaternion qNew(*this);
			qNew.SlerpSet(rhs, fDelta);
			return qNew;
		}

		FORCEINLINE IOEQuaternion Slerp(const IOEQuaternion &rhs,
										const IOEVector &vDelta)
		{
			IOEQuaternion qNew(*this);
			qNew.SlerpSet(rhs, vDelta);
			return qNew;
		}

		FORCEINLINE void SlerpSet(const IOEQuaternion &rhs, float fDelta)
		{
			m_qData = XMQuaternionSlerp(m_qData, m_qData, fDelta);
		}

		FORCEINLINE void SlerpSet(const IOEQuaternion &rhs,
								  const IOEVector &vDelta)
		{
			m_qData = XMQuaternionSlerpV(m_qData, m_qData, vDelta.GetRaw());
		}

	public:
		FORCEINLINE static IOEQuaternion Multiply(const IOEQuaternion &lhs,
												  const IOEQuaternion &rhs)
		{
			IOEQuaternion qNew(lhs);
			qNew.MultiplySet(rhs);
			return qNew;
		}

		FORCEINLINE IOEQuaternion Multiply(const IOEQuaternion &rhs) const
		{
			IOEQuaternion qNew(*this);
			qNew.MultiplySet(rhs);
			return qNew;
		}

		FORCEINLINE void MultiplySet(const IOEQuaternion &rhs)
		{
			m_qData = XMQuaternionMultiply(m_qData, rhs.m_qData);
		}

		FORCEINLINE IOEQuaternion operator*(const IOEQuaternion &rhs) const
		{
			return Multiply(rhs);
		}

		FORCEINLINE IOEQuaternion &operator*=(const IOEQuaternion &rhs)
		{
			MultiplySet(rhs);
			return *this;
		}

	public:
		FORCEINLINE IOEVector Rotate(const IOEVector &rhs) const
		{
			return IOEVector(XMVector3Rotate(rhs.GetRaw(), m_qData));
		}

	private:
		FORCEINLINE void Store(float &fX, float &fY, float &fZ, float &fW)
			const
		{
			XMFLOAT4 tData;
			XMStoreFloat4(&tData, m_qData);
			fX = tData.x;
			fY = tData.y;
			fZ = tData.z;
			fW = tData.w;
		}

		FORCEINLINE void Load(float fX, float fY, float fZ, float fW)
		{
			XMFLOAT4 tData(fX, fY, fZ, fW);
			m_qData = XMLoadFloat4(&tData);
		}
	};

	//////////////////////////////////////////////////////////////////////////

	extern IOEQuaternion g_qIdentity;

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE