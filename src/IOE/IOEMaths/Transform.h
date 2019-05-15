#pragma once

#include <cstdint>

#include <DirectXMath.h>

#include <IOE/IOECore/IOEDefines.h>

#include "Matrix.h"
#include "Vector.h"
#include "Quaternion.h"

using namespace DirectX;

//////////////////////////////////////////////////////////////////////////

namespace IOE
{
namespace Maths
{

	//////////////////////////////////////////////////////////////////////////

	enum class ERotationOrder : std::uint16_t
	{
		ERO_XYZ,
		ERO_XZY,
		ERO_YXZ,
		ERO_YZX,
		ERO_ZXY,
		ERO_ZYX,

		ERO_Default
	};

	//////////////////////////////////////////////////////////////////////////

	class alignas(16) IOETransform
	{
	public:
		IOETransform();
		IOETransform(const IOEVector &vTranslation,
					 const IOEQuaternion &qRotation,
					 const IOEVector &vScale = g_vOneVector);
		IOETransform(const IOEMatrix &mMat);

		static IOETransform GetRotationFromEulers(
			const IOEVector &vEulerRotation, ERotationOrder nRotation);

		explicit operator IOEMatrix() const;
		IOEMatrix ToMatrix() const;

		void FromMatrix(const IOEMatrix &rMat);

		bool operator==(const IOETransform &rhs) const;
		bool operator!=(const IOETransform &rhs) const;

		IOETransform Blend(const IOETransform &rOther, float fDelta);

		void SetScale(const IOEVector &vScale);
		void SetScale(float fX, float fY, float fZ);
		void GetScale(float &fX, float &fY, float &fZ) const;
		const IOEVector &GetScale() const;

		void SetTranslation(const IOEVector &vTranslation);
		void SetTranslation(float fX, float fY, float fZ);
		void GetTranslation(float &fX, float &fY, float &fZ) const;
		const IOEVector &GetTranslation() const;

		void SetRotationQuat(const IOEQuaternion &qRotation);
		const IOEQuaternion &GetRotationQuat() const;

		void SetRotation(float fPitch, float fYaw, float fRoll,
						 ERotationOrder nRotation =
							 ERotationOrder::ERO_Default);
		void GetRotation(float &fPitch, float &fYaw, float &fRoll) const;

		std::uint32_t IsDirty() const;

		IOEVector GetAxis(std::uint32_t nAxis) const;
		IOEVector GetColumn(std::uint32_t nCol) const;

	protected:
		void UpdateMatrix() const;

	private:
		mutable IOEMatrix m_mMat;

		IOEVector m_vScale;
		IOEVector m_vTranslation;
		IOEQuaternion m_qRotation;

#if (IOE_DEBUG_ENABLED == 1)
		IOEVector m_vEulerRotation;
#endif // (IOE_DEBUG_ENABLED == 1)

		mutable std::uint32_t m_bMatrixDirty;
	};

	//////////////////////////////////////////////////////////////////////////

} // namespace Maths
} // namespace IOE