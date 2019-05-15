#include "Transform.h"

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	IOETransform::IOETransform()
		: m_bMatrixDirty(false)
		, m_vScale(g_vOneVector)
		, m_vTranslation(g_vZeroVector)
		, m_qRotation(g_qIdentity)
#if (IOE_DEBUG_ENABLED == 1)
		, m_vEulerRotation(g_vZeroVector)
#endif // (IOE_DEBUG_ENABLED == 1)
		, m_mMat(g_mIdentity)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform::IOETransform(const IOEMatrix &mMat)
		: m_bMatrixDirty(false)
		, m_mMat(mMat)
	{
		FromMatrix(mMat);
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform::IOETransform(const IOEVector &vTranslation,
							   const IOEQuaternion &qRotation,
							   const IOEVector &vScale /*=g_vOneVector*/)
		: m_bMatrixDirty(true)
		, m_mMat(g_mIdentity)
		, m_vTranslation(vTranslation)
		, m_qRotation(qRotation)
		, m_vScale(vScale)
	{
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform::operator IOEMatrix() const
	{
		return ToMatrix();
	}

	//////////////////////////////////////////////////////////////////////////

	IOEMatrix IOETransform::ToMatrix() const
	{
		if (m_bMatrixDirty)
		{
			UpdateMatrix();
		}
		return m_mMat;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::FromMatrix(const IOEMatrix &rMat)
	{
		XMVECTOR qRot;
		XMVECTOR vTranslation;
		XMVECTOR vScale;

		m_mMat = rMat;

		XMMatrixDecompose(&vScale, &qRot, &vTranslation, m_mMat.GetRaw());

		m_qRotation	= qRot;
		m_vTranslation = vTranslation;
		m_vScale	   = vScale;

#if (IOE_DEBUG_ENABLED == 1)
		m_vEulerRotation =
			IOEVector(m_qRotation.GetPitch(), m_qRotation.GetYaw(),
					  m_qRotation.GetRoll());
#endif // (IOE_DEBUG_ENABLED == 1)

		m_bMatrixDirty = false;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetScale(const IOEVector &vScale)
	{
		m_vScale = vScale;

		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetScale(float fX, float fY, float fZ)
	{
		m_vScale.Set(fX, fY, fZ);

		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::GetScale(float &fX, float &fY, float &fZ) const
	{
		m_vScale.Get(fX, fY, fZ);
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEVector &IOETransform::GetScale() const
	{
		return m_vScale;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetTranslation(const IOEVector &vTranslation)
	{
		m_vTranslation = vTranslation;
		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetTranslation(float fX, float fY, float fZ)
	{
		m_vTranslation.Set(fX, fY, fZ);

		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::GetTranslation(float &fX, float &fY, float &fZ) const
	{
		m_vTranslation.Get(fX, fY, fZ);
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEVector &IOETransform::GetTranslation() const
	{
		return m_vTranslation;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetRotationQuat(const IOEQuaternion &qRotation)
	{
#if (IOE_DEBUG_ENABLED == 1)
		m_vEulerRotation = IOEVector(qRotation.GetPitch(), qRotation.GetYaw(),
									 qRotation.GetRoll());
#endif // (IOE_DEBUG_ENABLED == 1)

		m_qRotation	= qRotation;
		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	const IOEQuaternion &IOETransform::GetRotationQuat() const
	{
		return m_qRotation;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::SetRotation(
		float fPitch, float fYaw, float fRoll,
		ERotationOrder nRotation /*=ERotationOrder::ERO_Default*/)
	{
		IOEVector vEuler(fPitch, fYaw, fRoll, 0.0f);

#if (IOE_DEBUG_ENABLED == 1)
		m_vEulerRotation = vEuler;
#endif // (IOE_DEBUG_ENABLED == 1)

		if (nRotation == ERotationOrder::ERO_Default)
		{
			// Convert from degrees to radians
			m_qRotation = IOEQuaternion(fPitch, fYaw, fRoll);
		}
		else
		{
			IOEMatrix mRotX(IOEMatrix::RotationXDeg(fPitch));
			IOEMatrix mRotY(IOEMatrix::RotationYDeg(fYaw));
			IOEMatrix mRotZ(IOEMatrix::RotationZDeg(fRoll));
			IOEMatrix mRot(g_mIdentity);

			switch (nRotation)
			{
			case ERotationOrder::ERO_XYZ:
				mRot = mRotX * mRotY * mRotZ;
				break;
			case ERotationOrder::ERO_XZY:
				mRot = mRotX * mRotZ * mRotY;
				break;
			case ERotationOrder::ERO_YXZ:
				mRot = mRotY * mRotX * mRotZ;
				break;
			case ERotationOrder::ERO_YZX:
				mRot = mRotY * mRotZ * mRotX;
				break;
			case ERotationOrder::ERO_ZXY:
				mRot = mRotZ * mRotX * mRotY;
				break;
			case ERotationOrder::ERO_ZYX:
				mRot = mRotZ * mRotY * mRotX;
				break;
			}

			m_qRotation = IOEQuaternion(mRot);
		}

		m_bMatrixDirty = true;
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::GetRotation(float &fPitch, float &fYaw,
								   float &fRoll) const
	{
		fPitch = m_qRotation.GetPitch();
		fYaw   = m_qRotation.GetYaw();
		fRoll  = m_qRotation.GetRoll();
	}

	//////////////////////////////////////////////////////////////////////////

	void IOETransform::UpdateMatrix() const
	{
		if (m_bMatrixDirty)
		{
			m_bMatrixDirty = false;

			m_mMat = DirectX::XMMatrixAffineTransformation(
				m_vScale.GetRaw(), g_vIdentity.GetRaw(), m_qRotation.GetRaw(),
				m_vTranslation.GetRaw());
		}
	}

	//////////////////////////////////////////////////////////////////////////

	std::uint32_t IOETransform::IsDirty() const
	{
		return m_bMatrixDirty;
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform IOETransform::Blend(const IOETransform &rOther, float fDelta)
	{
		IOEVector vDelta(fDelta);

		IOETransform tBlend(*this);

		// Slerp rotation
		tBlend.m_qRotation.SlerpSet(rOther.GetRotationQuat(), vDelta);

		// Lerp scale and translation
		tBlend.m_vTranslation.LerpSet(rOther.GetTranslation(), vDelta);
		tBlend.m_vScale.LerpSet(rOther.GetScale(), vDelta);

		return tBlend;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETransform::operator==(const IOETransform &rhs) const
	{
		if (m_vTranslation != rhs.m_vTranslation)
		{
			return false;
		}
		if (m_vScale != rhs.m_vScale)
		{
			return false;
		}
		if (m_qRotation != rhs.m_qRotation)
		{
			return false;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool IOETransform::operator!=(const IOETransform &rhs) const
	{
		return !(operator==(rhs));
	}

	//////////////////////////////////////////////////////////////////////////

	IOEVector IOETransform::GetAxis(std::uint32_t nAxis) const
	{
		if (m_bMatrixDirty)
		{
			UpdateMatrix();
		}
		return m_mMat.GetColumn(nAxis);
	}

	//////////////////////////////////////////////////////////////////////////

	IOEVector IOETransform::GetColumn(std::uint32_t nCol) const
	{
		if (m_bMatrixDirty)
		{
			UpdateMatrix();
		}
		return m_mMat.Transpose().GetColumn(nCol);
	}

	//////////////////////////////////////////////////////////////////////////

	IOETransform
	IOETransform::GetRotationFromEulers(const IOEVector &vEulerRotation,
										ERotationOrder nRotation)
	{
		IOETransform tTransform;
		float fX, fY, fZ;
		vEulerRotation.Get(fX, fY, fZ);
		tTransform.SetRotation(fX, fY, fZ, nRotation);
		return tTransform;
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE